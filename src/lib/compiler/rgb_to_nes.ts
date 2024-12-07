export const rgb888_to_rgb222 = (
    r: number,
    g: number,
    b: number
): number => {
    var r2: number = (r >> 6) & 3;
    var g2: number = (g >> 6) & 3;
    var b2: number = (b >> 6) & 3;
    return (b2 << 4) | (g2 << 2) | r2;
}

export const rgb555_to_rgb222 = (
    r: number,
    g: number,
    b: number
): number => {
    var r2: number = (r >> 3) & 3;
    var g2: number = (g >> 3) & 3;
    var b2: number = (b >> 3) & 3;
    return (b2 << 4) | (g2 << 2) | r2;
}

export const rgb222_to_nes = (
    c: number
): number => {
    console.log(`rgb222_to_nes: c = ${c}`);
    switch(c) {
        case 0x00:      return 0x1D;
        case 0x01:      return 0x06;
        case 0x02:      return 0x17;
        case 0x03:      return 0x16;
        case 0x04:      return 0x19;
        case 0x05:      return 0x18;
        case 0x06:      return 0x17;
        case 0x07:      return 0x27;
        case 0x08:      return 0x2A;
        case 0x09:      return 0x29;
        case 0x0A:      return 0x28;
        case 0x0B:      return 0x27;
        case 0x0C:      return 0x2A;
        case 0x0D:      return 0x29;
        case 0x0E:      return 0x29;
        case 0x0F:      return 0x28;
        case 0x10:      return 0x01;
        case 0x11:      return 0x04;
        case 0x12:      return 0x15;
        case 0x13:      return 0x15;
        case 0x14:      return 0x1C;
        case 0x15:      return 0x00;
        case 0x16:      return 0x15;
        case 0x17:      return 0x26;
        case 0x18:      return 0x2B;
        case 0x19:      return 0x2A;
        case 0x1A:      return 0x10;
        case 0x1B:      return 0x26;
        case 0x1C:      return 0x2B;
        case 0x1D:      return 0x2A;
        case 0x1E:      return 0x39;
        case 0x1F:      return 0x38;
        case 0x20:      return 0x02;
        case 0x21:      return 0x13;
        case 0x22:      return 0x14;
        case 0x23:      return 0x14;
        case 0x24:      return 0x11;
        case 0x25:      return 0x13;
        case 0x26:      return 0x10;
        case 0x27:      return 0x25;
        case 0x28:      return 0x2C;
        case 0x29:      return 0x10;
        case 0x2A:      return 0x3D;
        case 0x2B:      return 0x36;
        case 0x2C:      return 0x2C;
        case 0x2D:      return 0x3B;
        case 0x2E:      return 0x3A;
        case 0x2F:      return 0x37;
        case 0x30:      return 0x12;
        case 0x31:      return 0x13;
        case 0x32:      return 0x14;
        case 0x33:      return 0x24;
        case 0x34:      return 0x12;
        case 0x35:      return 0x22;
        case 0x36:      return 0x23;
        case 0x37:      return 0x24;
        case 0x38:      return 0x21;
        case 0x39:      return 0x22;
        case 0x3A:      return 0x32;
        case 0x3B:      return 0x34;
        case 0x3C:      return 0x2C;
        case 0x3D:      return 0x3C;
        case 0x3E:      return 0x3C;
        case 0x3F:      return 0x20;
        default:        return 0xFF; // out-of-range value - set to 0xFF
    }
    return 0xFF;
}
