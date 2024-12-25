import { hexDec } from "shared/lib/helpers/8bit";

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

export const rgb888Hex_to_nesHex = (
  c: string,
): string => {
  const r: number = hexDec(c.substring(0, 2));
  const g: number = hexDec(c.substring(2, 4));
  const b: number = hexDec(c.substring(4, 6));
  return rgb222_to_nes(rgb888_to_rgb222(r, g, b)).toString(16).padStart(2, "0").toUpperCase();
}

export const rgb888_to_nes = (
  r: number,
  g: number,
  b: number
): number => {
  return rgb222_to_nes(rgb888_to_rgb222(r, g, b));
}

export const rgb888_to_nesHex = (
  r: number,
  g: number,
  b: number
): string => {
  return rgb888_to_nes(r, g, b).toString(16).padStart(2, "0").toUpperCase();
}

export const rgb555_to_nes = (
    r: number,
    g: number,
    b: number
): number => {
    return rgb222_to_nes(rgb555_to_rgb222(r, g, b));
}

export const rgb222_to_nes = (
    c: number
): number => {
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
}

export const nes_to_rgb222 = (
    c: number
): number => {
    switch (c) {
        case 0x1D: return 0x00;
        case 0x06: return 0x01;
        case 0x17: return 0x02;
        case 0x16: return 0x03;
        case 0x19: return 0x04;
        case 0x18: return 0x05;
        case 0x17: return 0x06;
        case 0x27: return 0x07;
        case 0x2A: return 0x08;
        case 0x29: return 0x09;
        case 0x28: return 0x0A;
        case 0x27: return 0x0B;
        case 0x2A: return 0x0C;
        case 0x29: return 0x0D;
        case 0x29: return 0x0E;
        case 0x28: return 0x0F;
        case 0x01: return 0x10;
        case 0x04: return 0x11;
        case 0x15: return 0x12;
        case 0x15: return 0x13;
        case 0x1C: return 0x14;
        case 0x00: return 0x15;
        case 0x15: return 0x16;
        case 0x26: return 0x17;
        case 0x2B: return 0x18;
        case 0x2A: return 0x19;
        case 0x10: return 0x1A;
        case 0x26: return 0x1B;
        case 0x2B: return 0x1C;
        case 0x2A: return 0x1D;
        case 0x39: return 0x1E;
        case 0x38: return 0x1F;
        case 0x02: return 0x20;
        case 0x13: return 0x21;
        case 0x14: return 0x22;
        case 0x14: return 0x23;
        case 0x11: return 0x24;
        case 0x13: return 0x25;
        case 0x10: return 0x26;
        case 0x25: return 0x27;
        case 0x2C: return 0x28;
        case 0x10: return 0x29;
        case 0x3D: return 0x2A;
        case 0x36: return 0x2B;
        case 0x2C: return 0x2C;
        case 0x3B: return 0x2D;
        case 0x3A: return 0x2E;
        case 0x37: return 0x2F;
        case 0x12: return 0x30
        case 0x13: return 0x31;
        case 0x14: return 0x32;
        case 0x24: return 0x33;
        case 0x12: return 0x34;
        case 0x22: return 0x35;
        case 0x23: return 0x36;
        case 0x24: return 0x37;
        case 0x21: return 0x38;
        case 0x22: return 0x39;
        case 0x32: return 0x3A;
        case 0x34: return 0x3B;
        case 0x2C: return 0x3C;
        case 0x3C: return 0x3D;
        case 0x3C: return 0x3E;
        case 0x20: return 0x3F;
        default:   return 0xFF; // out-of-range value - set to 0xFF
    }
}

export const nes_to_rgb555 = (
    c: number
): number => {
    const rgb222: number = nes_to_rgb222(c);
    const r: number = (rgb222 >> 0) & 3;
    const g: number = (rgb222 >> 2) & 3;
    const b: number = (rgb222 >> 4) & 3;
    const rgb555: number = (b << 13) | (g << 7) | (r << 3);
    return rgb555;
}

export const nes_to_rgb888 = (
  c: number
): number => {
  const rgb222: number = nes_to_rgb222(c);
  const r: number = (rgb222 >> 0) & 3;
  const g: number = (rgb222 >> 2) & 3;
  const b: number = (rgb222 >> 4) & 3;
  const rgb888: number = (b << 13) | (g << 7) | (r << 3);
  return rgb888;
}

const nes_ppu_rgb: string[] = [
  // 00-0F
  "626262",
  "012090",
  "240BA0",
  "470090",
  "600062",
  "6A0024",
  "601100",
  "472700",
  "243C00",
  "014A00",
  "004F00",
  "004724",
  "003662",
  "000000",
  "000000",
  "000000",
  // 10-1F
  "ABABAB",
  "1F56E1",
  "4D39FF",
  "7E23EF",
  "A31BB7",
  "B42264",
  "AC370E",
  "8C5500",
  "5E7200",
  "2D8800",
  "079000",
  "008947",
  "00739D",
  "000000",
  "000000",
  "000000",
  // 20-2F
  "FFFFFF",
  "67ACFF",
  "958DFF",
  "C875FF",
  "F26AFF",
  "FF6FC5",
  "FF836A",
  "E6A01F",
  "B8BF00",
  "85D801",
  "5BE335",
  "45DE88",
  "49CAE3",
  "4E4E4E",
  "000000",
  "000000",
  // 30-3F
  "FFFFFF",
  "BFE0FF",
  "D1D3FF",
  "E6C9FF",
  "F7C3FF",
  "FFC4EE",
  "FFCBC9",
  "F7D7A9",
  "E6E397",
  "D1EE97",
  "BFF3A9",
  "B5F2C9",
  "B5EBEE",
  "B8B8B8",
  "000000",
  "000000"
];

export const nesHex_to_rgb888Hex = (
  h: string,
): string => {
  const c: number = hexDec(h.substring(0, 2));
  return nes_ppu_rgb[c];
}

export const nesHex_to_rgb888_struct = (
  h: string,
) => {
  const c: number = hexDec(h.substring(0, 2));
  const rgbHex: string = nes_ppu_rgb[c];
  const r: number = hexDec(rgbHex.substring(0, 2));
  const g: number = hexDec(rgbHex.substring(2, 4));
  const b: number = hexDec(rgbHex.substring(4, 6));
  return { r, g, b };
}