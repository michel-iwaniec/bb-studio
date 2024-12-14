import l10n from "shared/lib/lang/l10n";
import { divisibleBy8 } from "shared/lib/helpers/8bit";
import type {
  BackgroundData,
  Tileset,
} from "shared/lib/entities/entitiesTypes";
import {
  toTileLookup,
  tilesAndLookupToTilemap,
} from "shared/lib/tiles/tileData";
import { assetFilename } from "shared/lib/helpers/assets";
import { readFileToTilesDataArray } from "lib/tiles/readFileToTiles";
import { MAX_BACKGROUND_TILES, MAX_BACKGROUND_TILES_CGB } from "consts";
import { SCREEN_WIDTH_PX, SCREEN_HEIGHT_PX } from "consts";

const MAX_IMAGE_WIDTH = 2040;
const MAX_IMAGE_HEIGHT = 2040;
const MAX_PIXELS = 16380 * 64;

export interface BackgroundInfo {
  numTiles: number;
  warnings: string[];
  lookup: number[];
}

const mergeCommonTiles = async (
  tileData: Uint8Array[],
  commonTileset: Tileset | undefined,
  projectPath: string
) => {
  if (!commonTileset) {
    return tileData;
  }
  const commonFilename = assetFilename(projectPath, "tilesets", commonTileset);
  const commonTileData = await readFileToTilesDataArray(commonFilename);
  return [...commonTileData, ...tileData];
};

export const getBackgroundInfo = async (
  background: BackgroundData,
  commonTileset: Tileset | undefined,
  is360: boolean,
  isCGBOnly: boolean,
  projectPath: string,
  precalculatedTilesetLength?: number
): Promise<BackgroundInfo> => {
  const warnings: string[] = [];

  let tilesetLength = precalculatedTilesetLength;
  let tilesets: number[] = [];
  if (!tilesetLength) {
    const filename = assetFilename(projectPath, "backgrounds", background);
    const tileData = await readFileToTilesDataArray(filename);
    const tileDataWithCommon = await mergeCommonTiles(
      tileData,
      commonTileset,
      projectPath
    );
    const tilesetLookup = toTileLookup(tileDataWithCommon);
    tilesets = tilesAndLookupToTilemap(tileData, tilesetLookup);
    tilesetLength = Object.keys(tilesetLookup).length;
  }

  if (background.imageWidth < 160 || background.imageHeight < 144) {
    warnings.push(l10n("WARNING_BACKGROUND_TOO_SMALL"));
  }
  if (background.imageWidth > MAX_IMAGE_WIDTH) {
    warnings.push(
      l10n("WARNING_BACKGROUND_TOO_WIDE", {
        width: background.imageWidth,
        maxWidth: MAX_IMAGE_WIDTH,
      })
    );
  }
  if (background.imageHeight > MAX_IMAGE_HEIGHT) {
    warnings.push(
      l10n("WARNING_BACKGROUND_TOO_TALL", {
        height: background.imageHeight,
        maxHeight: MAX_IMAGE_HEIGHT,
      })
    );
  }
  if (background.imageWidth * background.imageHeight > MAX_PIXELS) {
    warnings.push(
      l10n("WARNING_BACKGROUND_TOO_MANY_PIXELS", {
        width: background.imageWidth,
        height: background.imageHeight,
        numPixels: background.imageWidth * background.imageHeight,
        maxPixels: MAX_PIXELS,
      })
    );
  }
  if (
    !divisibleBy8(background.imageWidth) ||
    !divisibleBy8(background.imageHeight)
  ) {
    warnings.push(l10n("WARNING_BACKGROUND_NOT_MULTIPLE_OF_8"));
  }
  if (tilesetLength > MAX_BACKGROUND_TILES && !is360 && !isCGBOnly) {
    warnings.push(
      l10n("WARNING_BACKGROUND_TOO_MANY_TILES", {
        tilesetLength,
        maxTilesetLength: MAX_BACKGROUND_TILES,
      })
    );
  }

  if (tilesetLength > MAX_BACKGROUND_TILES && !is360 && isCGBOnly) {
    warnings.push(
      l10n("WARNING_BACKGROUND_TOO_MANY_TILES", {
        tilesetLength,
        maxTilesetLength: MAX_BACKGROUND_TILES,
      })
    );
  }

  if (
    is360 &&
    (background.imageWidth !== SCREEN_WIDTH_PX || background.imageHeight !== SCREEN_HEIGHT_PX)
  ) {
    warnings.push(
      l10n("WARNING_LOGO_WRONG_SIZE", {
        width: background.imageWidth,
        height: background.imageHeight,
      })
    );
  }
  return {
    warnings,
    numTiles: tilesetLength,
    lookup: tilesets,
  };
};
