void kernel_main(void)
{
    volatile unsigned char *video =
        (volatile unsigned char *)0xB8000;

    video[0] = 'K';
    video[1] = 0x07;

    video[2] = 'A';
    video[3] = 0x07;

    video[4] = 'N';
    video[5] = 0x07;

    video[6] = 'H';
    video[7] = 0x07;

    video[8] = 'A';
    video[9] = 0x07;

    while (1) {
    }
}