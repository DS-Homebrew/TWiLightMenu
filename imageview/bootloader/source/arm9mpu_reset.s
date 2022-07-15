	.arm
	.global mpu_reset, mpu_reset_end

mpu_reset:
	.incbin "mpu_reset.bin"
mpu_reset_end:
