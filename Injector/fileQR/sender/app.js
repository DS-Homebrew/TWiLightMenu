// fileQR sender — ferramenta de transferência (protocolo FQR1 binário).
// Depende de vendor-qrcode.js (qrcode-generator, Kazuhiko Arase, Byte mode Latin-1).
'use strict';

// ---- CRC32 (poly 0xEDB88320, igual ao crc32.c do lado NDS) ----
const CRC_TABLE = (() => {
  const t = new Uint32Array(256);
  for (let n = 0; n < 256; n++) {
    let c = n;
    for (let k = 0; k < 8; k++) c = (c & 1) ? (0xEDB88320 ^ (c >>> 1)) : (c >>> 1);
    t[n] = c >>> 0;
  }
  return t;
})();
function crc32(bytes) {
  let c = 0xFFFFFFFF;
  for (let i = 0; i < bytes.length; i++) c = CRC_TABLE[(c ^ bytes[i]) & 0xFF] ^ (c >>> 8);
  return (c ^ 0xFFFFFFFF) >>> 0;
}

// ---- escrita binária little-endian ----
function u16(v) { return [v & 0xFF, (v >>> 8) & 0xFF]; }
function u32(v) { return [v & 0xFF, (v >>> 8) & 0xFF, (v >>> 16) & 0xFF, (v >>> 24) & 0xFF]; }
const MAGIC = [0x46, 0x51, 0x52, 0x31]; // "FQR1"

function bytesToLatin1(bytes) {
  let s = '';
  const CH = 8192;
  for (let i = 0; i < bytes.length; i += CH)
    s += String.fromCharCode.apply(null, bytes.subarray(i, Math.min(bytes.length, i + CH)));
  return s;
}

function buildFrames(fileBytes, filename, chunk) {
  const size = fileBytes.length;
  const total = Math.max(1, Math.ceil(size / chunk));
  const crc = crc32(fileBytes);
  const nameBytes = new TextEncoder().encode(filename).slice(0, 255);
  const manifest = Uint8Array.from([
    ...MAGIC, 0x00, ...u16(0), ...u16(total),
    ...u32(crc), ...u32(size), ...u16(chunk),
    nameBytes.length, ...nameBytes,
  ]);
  const frames = [manifest];
  for (let i = 0; i < total; i++) {
    const slice = fileBytes.subarray(i * chunk, Math.min(size, (i + 1) * chunk));
    const pkt = new Uint8Array(9 + slice.length);
    pkt.set([...MAGIC, 0x01, ...u16(i + 1), ...u16(total)], 0);
    pkt.set(slice, 9);
    frames.push(pkt);
  }
  return { frames, total, crc, size };
}

function buildOrder(total, manifestEvery) {
  const order = [0];
  for (let d = 1; d <= total; d++) {
    order.push(d);
    if (manifestEvery > 0 && d % manifestEvery === 0 && d !== total) order.push(0);
  }
  return order;
}

// ---- QR ----
function makeQR(bytes, ecc) {
  const qr = qrcode(0, ecc);
  qr.addData(bytesToLatin1(bytes)); // Byte mode Latin-1 = bytes exatos
  qr.make();
  return qr;
}
function renderQR(canvas, qr, scale) {
  const count = qr.getModuleCount();
  const quiet = 4;
  const dim = (count + quiet * 2) * scale;
  canvas.width = dim; canvas.height = dim;
  const ctx = canvas.getContext('2d');
  ctx.fillStyle = '#fff'; ctx.fillRect(0, 0, dim, dim);
  ctx.fillStyle = '#000';
  for (let r = 0; r < count; r++)
    for (let c = 0; c < count; c++)
      if (qr.isDark(r, c)) ctx.fillRect((c + quiet) * scale, (r + quiet) * scale, scale, scale);
  return count;
}
function fitScale(count, maxPx) {
  return Math.max(2, Math.floor(maxPx / (count + 8)));
}

// ---- helpers de formato ----
const $ = id => document.getElementById(id);
function fmtTime(s) {
  s = Math.round(s);
  if (s < 60) return `${s}s`;
  const m = Math.floor(s / 60); return `${m}m${String(s % 60).padStart(2, '0')}s`;
}
function fmtSize(b) {
  if (b < 1024) return `${b} B`;
  if (b < 1048576) return `${(b / 1024).toFixed(1)} KB`;
  return `${(b / 1048576).toFixed(2)} MB`;
}

// ---- estado ----
let state = { frames: [], order: [], pos: 0, total: 0, timer: null };
let presenting = false;

function stop() {
  if (state.timer) { clearInterval(state.timer); state.timer = null; }
  $('play').disabled = !state.order.length; $('pause').disabled = true;
}

function showPos(p) {
  if (!state.order.length) return;
  state.pos = ((p % state.order.length) + state.order.length) % state.order.length;
  const frameIdx = state.order[state.pos];
  const ecc = $('ecc').value;
  const qr = makeQR(state.frames[frameIdx], ecc);
  const count = qr.getModuleCount();
  const kind = frameIdx === 0 ? 'MANIFEST' : `dados ${frameIdx}/${state.total}`;
  const label = `seq ${state.pos + 1}/${state.order.length} · ${kind} · QR ${count}×${count}`;
  const pct = state.order.length > 1 ? (state.pos / (state.order.length - 1)) * 100 : 100;

  if (presenting) {
    const maxPx = Math.min(window.innerWidth, window.innerHeight) * 0.8;
    renderQR($('qrBig'), qr, fitScale(count, maxPx));
    $('hudText').textContent = label;
    $('barBig').style.width = pct + '%';
  } else {
    renderQR($('qr'), qr, parseInt($('scale').value, 10));
    $('frameLabel').textContent = label;
    $('bar').style.width = pct + '%';
    $('seek').value = state.pos;
  }
}

function play() {
  if (!state.order.length) return;
  stop();
  const fps = Math.max(1, parseInt($('fps').value, 10));
  $('play').disabled = true; $('pause').disabled = false;
  state.timer = setInterval(() => {
    let next = state.pos + 1;
    if (next >= state.order.length) {
      if ($('loop').checked) next = 0; else { stop(); return; }
    }
    showPos(next);
  }, 1000 / fps);
}

// ---- seleção de arquivo (clique + drag&drop) ----
let currentFile = null;
function setFile(f) {
  currentFile = f || null;
  $('build').disabled = !currentFile;
  if (currentFile)
    $('info').textContent = `Selecionado: ${currentFile.name}\nTamanho: ${fmtSize(currentFile.size)}\nClique em "Gerar frames".`;
}
$('file').addEventListener('change', e => setFile(e.target.files[0]));
const drop = $('drop');
drop.addEventListener('click', () => $('file').click());
['dragenter', 'dragover'].forEach(ev => drop.addEventListener(ev, e => {
  e.preventDefault(); drop.classList.add('over');
}));
['dragleave', 'drop'].forEach(ev => drop.addEventListener(ev, e => {
  e.preventDefault(); drop.classList.remove('over');
}));
drop.addEventListener('drop', e => { if (e.dataTransfer.files[0]) setFile(e.dataTransfer.files[0]); });

// ---- presets ----
const PRESETS = {
  seguro:      { chunk: 256, ecc: 'Q', fps: 3, manifestEvery: 20, scale: 8 },
  equilibrado: { chunk: 512, ecc: 'M', fps: 5, manifestEvery: 40, scale: 6 },
  rapido:      { chunk: 900, ecc: 'L', fps: 8, manifestEvery: 60, scale: 5 },
};
document.querySelectorAll('[data-preset]').forEach(b => b.addEventListener('click', () => {
  const p = PRESETS[b.dataset.preset];
  $('chunk').value = p.chunk; $('ecc').value = p.ecc; $('fps').value = p.fps;
  $('manifestEvery').value = p.manifestEvery; $('scale').value = p.scale;
  if (state.order.length) $('info').textContent += '\n(preset alterado — clique "Gerar frames")';
}));

// ---- gerar ----
$('build').addEventListener('click', async () => {
  if (!currentFile) return;
  const buf = new Uint8Array(await currentFile.arrayBuffer());
  const chunk = Math.max(32, parseInt($('chunk').value, 10));
  const fps = Math.max(1, parseInt($('fps').value, 10));
  const manifestEvery = Math.max(0, parseInt($('manifestEvery').value, 10));

  const t0 = performance.now();
  const { frames, total, crc, size } = buildFrames(buf, currentFile.name, chunk);
  const order = buildOrder(total, manifestEvery);
  state = { frames, order, pos: 0, total, timer: null };

  $('seek').max = order.length - 1; $('seek').value = 0; $('seek').disabled = false;
  $('play').disabled = false; $('pause').disabled = true; $('present').disabled = false;

  const estTime = order.length / fps;
  const rate = chunk * fps;
  const ext = (currentFile.name.split('.').pop() || '').toLowerCase();
  const ndsNote = ext === 'nds' ? '\n(.nds — no DSi escolha a pasta e START antes de filmar)' : '';
  $('info').textContent =
    `Arquivo : ${currentFile.name}\n` +
    `Tamanho : ${fmtSize(size)} (${size} B)\n` +
    `CRC32   : ${crc.toString(16).padStart(8, '0')}\n` +
    `Chunk   : ${chunk} B  ·  QR binário\n` +
    `Frames  : ${total} dados + reexibições = ${order.length} na fila\n` +
    `A ${fps} fps: ~${fmtTime(estTime)}/volta  ·  ~${fmtSize(rate)}/s\n` +
    `Gerado em ${(performance.now() - t0).toFixed(0)} ms${ndsNote}`;
  showPos(0);
});

$('play').addEventListener('click', play);
$('pause').addEventListener('click', stop);
$('seek').addEventListener('input', e => { stop(); showPos(parseInt(e.target.value, 10)); });
['ecc', 'scale'].forEach(id => $(id).addEventListener('change', () => showPos(state.pos)));

// ---- apresentação em tela cheia ----
async function enterPresent() {
  if (!state.order.length) return;
  presenting = true;
  $('stagefull').classList.remove('hidden');
  try { await $('stagefull').requestFullscreen(); } catch (e) { /* file:// pode negar; overlay já cobre */ }
  showPos(state.pos);
  if (!state.timer) play();
}
function exitPresent() {
  presenting = false;
  $('stagefull').classList.add('hidden');
  if (document.fullscreenElement) document.exitFullscreen().catch(() => {});
  showPos(state.pos);
}
$('present').addEventListener('click', enterPresent);
document.addEventListener('fullscreenchange', () => {
  if (!document.fullscreenElement && presenting) exitPresent();
});
window.addEventListener('resize', () => { if (presenting) showPos(state.pos); });
document.addEventListener('keydown', e => {
  if (e.key === ' ') { e.preventDefault(); state.timer ? stop() : play(); }
  else if (e.key === 'Escape' && presenting) exitPresent();
});
