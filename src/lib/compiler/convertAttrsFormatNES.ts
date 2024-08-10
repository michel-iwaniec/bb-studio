
export interface AttrMap {
  data: Uint8Array;
  width: number;
  height: number;
}

const ReduceMapAttributes2x2 = (map: AttrMap, xo: number, yo: number): AttrMap => { 
    var attrs = map.data;
    var width = map.width;
    var height = map.height;
    var w = Math.trunc((width + 1) / 2);
    var h = Math.trunc((height + 1) / 2);
    var map_attributes_2x2 = new Uint8Array(w * h).fill(0);

    for(let y = 0; y < h; y++)
    {
        for(let x = 0; x < w; x++)
        {
            // Use only Top-left attribute, ignoring the other three as they should now be identical
            map_attributes_2x2[y * w + x] = attrs[(2 * y + yo) * width + 2 * x + xo];
        }
    }
    return { data: map_attributes_2x2, width: w, height: h };
}


//
// Aligns map attribute data to be aligned properly for NES and set_bkg_submap_attributes
// Namely:
// * Width aligned to multiples of 2 to reflect the NES's packed attribute table
// * Every 16th row is blank to reflect the unused row in the NES's packed attribute table
//
const AlignMapAttributes = (map: AttrMap): AttrMap => {
    var attrs = map.data;
    var w: number = map.width;
    var h: number = map.height;
    const ATTRIBUTE_HEIGHT: number = 15;
    const ATTRIBUTE_ALIGNED_HEIGHT: number = 16;
    var map_attributes_aligned_width = 2 * Math.trunc((w + 1) / 2);
    var num_vertical_nametables = Math.trunc((h + ATTRIBUTE_HEIGHT - 1) / ATTRIBUTE_HEIGHT);
    var map_attributes_aligned = new Uint8Array(map_attributes_aligned_width * (num_vertical_nametables * ATTRIBUTE_ALIGNED_HEIGHT)).fill(0);
    for(let i = 0; i < num_vertical_nametables; i++)
    {
        var last_nametable: boolean = (i == num_vertical_nametables - 1);
        var height = last_nametable ? (h - i * ATTRIBUTE_HEIGHT) : ATTRIBUTE_HEIGHT;
        for(let y = 0; y < height; y++)
        {
            for(let x = 0; x < w; x++)
            {
                map_attributes_aligned[(i * ATTRIBUTE_ALIGNED_HEIGHT + y) * map_attributes_aligned_width + x] =
                    attrs[(i * ATTRIBUTE_HEIGHT + y) * w + x];
            }
        }
    }
    return { data: map_attributes_aligned,
             width: map_attributes_aligned_width,
             height: num_vertical_nametables * ATTRIBUTE_ALIGNED_HEIGHT };
}

//
// Pack map attributes
// (NES packs multiple 2-bit entries into one byte)
//
const PackMapAttributes = (map: AttrMap): AttrMap => {
    var width = Math.trunc((map.width + 1) / 2);
    var height = Math.trunc((map.height + 1) / 2);
    var map_attributes_packed = new Uint8Array(width * height).fill(0);

    for(let y = 0; y < height; y++)
    {
        for(let x = 0; x < width; x++)
        {
            var a_tl = map.data[(2 * y + 0) * map.width + (2 * x + 0)] & 0x3;
            var a_tr = map.data[(2 * y + 0) * map.width + (2 * x + 1)] & 0x3;
            var a_bl = map.data[(2 * y + 1) * map.width + (2 * x + 0)] & 0x3;
            var a_br = map.data[(2 * y + 1) * map.width + (2 * x + 1)] & 0x3;
            var packed_bits = (a_br << 6) | (a_bl << 4) | (a_tr << 2) | (a_tl << 0);
            map_attributes_packed[width * y + x] = packed_bits;
        }
    }
    return { data: map_attributes_packed, width: width, height: height };
}

const MergePackedAttributes = (map_TL: AttrMap, map_TR: AttrMap, map_BL: AttrMap, map_BR: AttrMap): AttrMap => {
    var width: number = map_TL.width;
    var height: number = map_TL.height;
    var map_size: number = width * height;
    var map_attributes_merged = new Uint8Array(4 * map_size).fill(0);
    for(let y = 0; y < height; y++)
    {
        for(let x = 0; x < width; x++)
        {
            // Pack TL map, followed by TR map
            map_attributes_merged[0 * map_size + width * y + x] = map_TL.data[width * y + x];
            map_attributes_merged[1 * map_size + width * y + x] = map_TR.data[width * y + x];
            map_attributes_merged[2 * map_size + width * y + x] = map_BL.data[width * y + x];
            map_attributes_merged[3 * map_size + width * y + x] = map_BR.data[width * y + x];
        }
    }
    return { data: map_attributes_merged, width: width, height: height };
}

//  tilemapAttr: {
//    symbol: "ta_1",
//    data: new Uint8Array(),
//  },


// 
// export type Background = {
//   id: string;
//   name: string;
//   symbol: string;
//   filename: string;
//   width: number;
//   height: number;
//   imageWidth: number;
//   imageHeight: number;
//   tileColors: number[];
//   monoOverrideId?: string;
//   autoColor?: boolean;
//   plugin?: string;
//   inode: string;
//   _v: number;
// };
// 
// export type BackgroundData = Omit<Background, "_v" | "inode">;
// 

//void HandleMapAttributes(PNG2AssetData* assetData) {
//convertAttrsFormatNES(attrs: Uint8Array) {
const convertAttrsFormatNES = (attrs: number[] | Uint8Array, w: number, h: number): AttrMap => { 
    var width = w;
    var height = h;
    // Perform 2x2 reduction on attributes (NES attribute table has this format)
    // NES attribute map dimensions are half-resolution 
    var attr_2x2_TL = ReduceMapAttributes2x2(
        {data: new Uint8Array(attrs),
         width: width,
         height: height},
         0, 0
    );
    var attr_2x2_TR = ReduceMapAttributes2x2(
        {data: new Uint8Array(attrs),
         width: width,
         height: height},
         1, 0
    );
    var attr_2x2_BL = ReduceMapAttributes2x2(
        {data: new Uint8Array(attrs),
         width: width,
         height: height},
         0, 1
    );
    var attr_2x2_BR = ReduceMapAttributes2x2(
        {data: new Uint8Array(attrs),
         width: width,
         height: height},
         1, 1
    );
    // Align and pack map attributes into NES PPU format
    var attr_2x2_aligned_TL = AlignMapAttributes(attr_2x2_TL);
    var attr_2x2_aligned_TR = AlignMapAttributes(attr_2x2_TR);
    var attr_2x2_aligned_BL = AlignMapAttributes(attr_2x2_BL);
    var attr_2x2_aligned_BR = AlignMapAttributes(attr_2x2_BR);
    var attr_2x2_packed_TL = PackMapAttributes(attr_2x2_aligned_TL);
    var attr_2x2_packed_TR = PackMapAttributes(attr_2x2_aligned_TR);
    var attr_2x2_packed_BL = PackMapAttributes(attr_2x2_aligned_BL);
    var attr_2x2_packed_BR = PackMapAttributes(attr_2x2_aligned_BR);
    var merged_attributes = MergePackedAttributes(attr_2x2_packed_TL, 
                                                  attr_2x2_packed_TR,
                                                  attr_2x2_packed_BL,
                                                  attr_2x2_packed_BR);
    return merged_attributes;
}

export default convertAttrsFormatNES;
