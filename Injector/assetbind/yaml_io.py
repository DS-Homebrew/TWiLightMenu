"""
yaml_io.py — leitura/escrita do manifesto .yml.

Usa PyYAML se estiver instalado (lida bem com edições manuais). Caso contrário, usa um
leitor/escritor próprio, restrito ao schema deste projeto (version / games[] com
identity{} e assets{}). O round-trip do formato emitido é coberto pelos testes.
"""
import re


# --------------------------- escrita ---------------------------

def _q(s):
    """Aspas (obrigatórias para caminhos com espaços e para hashes)."""
    s = str(s).replace("\\", "\\\\").replace('"', '\\"')
    return f'"{s}"'


def dump_manifest(m):
    """Serializa o dict do manifesto em texto YAML determinístico."""
    out = [f"version: {int(m.get('version', 1))}"]
    if m.get("allow_name_match"):
        out.append("allow_name_match: true")
    out.append("games:")
    for g in m.get("games", []):
        idn = g.get("identity", {})
        a = g.get("assets", {})
        out.append(f"  - game_id: {_q(g['game_id'])}")
        out.append("    identity:")
        out.append(f"      sha1: {_q(idn.get('sha1', ''))}")
        out.append(f"      md5: {_q(idn.get('md5', ''))}")
        out.append(f"      crc32: {_q(idn.get('crc32', ''))}")
        out.append(f"      size: {int(idn.get('size', 0))}")
        if g.get("rom_name"):
            out.append(f"    rom_name: {_q(g['rom_name'])}")
        out.append("    assets:")
        out.append(f"      logo: {_q(a['logo']) if a.get('logo') else 'null'}")
        out.append(f"      video: {_q(a['video']) if a.get('video') else 'null'}")
        out.append(f"      video_top: {_q(a['video_top']) if a.get('video_top') else 'null'}")
        out.append(f"      video_bottom: {_q(a['video_bottom']) if a.get('video_bottom') else 'null'}")
    return "\n".join(out) + "\n"


def dump_assets_index(roms):
    """
    Serializa o assets_index.yml: mapa nome-base-da-ROM (sem extensão) -> game_id.
    É o índice runtime que o DS usa para casar a ROM em foco (por nome) e carregar
    assets/<game_id>/. Inclui TODOS os nomes (duplicatas apontam para o mesmo game_id).
    `roms`: dict {rom_base_name: game_id}.
    """
    out = [
        "# GERADO pelo host: nome-base-no-SD (sem extensão) -> game_id.",
        "# O DS casa a ROM em foco por aqui e carrega assets/<game_id>/.",
        "version: 1",
        "roms:",
    ]
    for name in sorted(roms):
        out.append(f"  {_q(name)}: {_q(roms[name])}")
    return "\n".join(out) + "\n"


# --------------------------- leitura ---------------------------

def _parse_scalar(v):
    v = v.strip()
    if v in ("", "null", "~"):
        return None
    if v == "true":
        return True
    if v == "false":
        return False
    if len(v) >= 2 and v[0] == '"' and v[-1] == '"':
        return v[1:-1].replace('\\"', '"').replace("\\\\", "\\")
    if re.fullmatch(r"-?\d+", v):
        return int(v)
    return v


def _split_kv(line):
    if ": " in line:
        k, v = line.split(": ", 1)
        return k.strip(), v.strip()
    if line.endswith(":"):
        return line[:-1].strip(), ""
    k, v = line.split(":", 1)
    return k.strip(), v.strip()


def _load_scoped(text):
    m = {"version": 1, "games": []}
    cur = None   # jogo atual
    sub = None   # mapa aninhado atual (identity/assets)
    for raw in text.splitlines():
        if not raw.strip() or raw.lstrip().startswith("#"):
            continue
        indent = len(raw) - len(raw.lstrip(" "))
        line = raw.strip()
        if indent == 0:
            k, v = _split_kv(line)
            if k == "games":
                cur = sub = None
                continue
            m[k] = _parse_scalar(v)
            cur = sub = None
        elif indent == 2 and line.startswith("- "):
            cur = {}
            sub = None
            m["games"].append(cur)
            k, v = _split_kv(line[2:])
            cur[k] = _parse_scalar(v)
        elif indent == 4 and cur is not None:
            k, v = _split_kv(line)
            if v == "":
                cur[k] = {}
                sub = cur[k]
            else:
                cur[k] = _parse_scalar(v)
                sub = None
        elif indent == 6 and sub is not None:
            k, v = _split_kv(line)
            sub[k] = _parse_scalar(v)
    return m


def load_manifest_text(text):
    try:
        import yaml  # type: ignore
        return yaml.safe_load(text)
    except ImportError:
        return _load_scoped(text)


def _parse_quoted_key_line(line):
    """Parseia `"chave com espaços": valor` (chave entre aspas). Retorna (key, value)."""
    if line.startswith('"'):
        end = 1
        while end < len(line):
            if line[end] == "\\":
                end += 2
                continue
            if line[end] == '"':
                break
            end += 1
        key = line[1:end].replace('\\"', '"').replace("\\\\", "\\")
        rest = line[end + 1:].lstrip()
        val = rest[1:].strip() if rest.startswith(":") else ""
        return key, _parse_scalar(val)
    k, v = _split_kv(line)
    return _parse_scalar(k) if k.startswith('"') else k, _parse_scalar(v)


def load_assets_index_text(text):
    """Lê o assets_index.yml: {version, roms: {nome: game_id}}."""
    try:
        import yaml  # type: ignore
        return yaml.safe_load(text)
    except ImportError:
        m = {"version": 1, "roms": {}}
        for raw in text.splitlines():
            line = raw.strip()
            if not line or line.startswith("#"):
                continue
            indent = len(raw) - len(raw.lstrip(" "))
            if indent == 0:
                k, v = _split_kv(line)
                if k == "roms":
                    continue
                m[k] = _parse_scalar(v)
            elif indent == 2:
                key, val = _parse_quoted_key_line(line)
                m["roms"][key] = val
        return m
