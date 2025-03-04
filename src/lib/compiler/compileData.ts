import keyBy from "lodash/keyBy";
import { eventHasArg } from "lib/helpers/eventSystem";
import compileImages from "./compileImages";
import compileEntityEvents from "./compileEntityEvents";
import {
  projectTemplatesRoot,
  MAX_ACTORS,
  MAX_TRIGGERS,
  DMG_PALETTE,
  MAX_NESTED_SCRIPT_DEPTH,
  MAX_PROJECTILES,
  EVENT_MUSIC_PLAY,
  EVENT_END,
  EVENT_PLAYER_SET_SPRITE,
  EVENT_ACTOR_SET_SPRITE,
} from "consts";
import compileSprites from "./compileSprites";
import compileAvatars from "./compileAvatars";
import compileEmotes from "./compileEmotes";
import compileFonts from "./compileFonts";
import convertAttrsFormatNES from "./convertAttrsFormatNES"
import {
  compileBackground,
  compileBackgroundHeader,
  compileTilemap,
  compileTilemapHeader,
  compileTilemapAttr,
  compileTilemapAttrHeader,
  compileTilemapAttrDummy,
  compileScene,
  compileSceneActors,
  compileSceneActorsHeader,
  compileSceneHeader,
  compileSceneTriggers,
  compileSceneTriggersHeader,
  compileSceneSprites,
  compileSceneSpritesHeader,
  compileSceneCollisions,
  compileSceneCollisionsHeader,
  compileSpriteSheet,
  compileSpriteSheetHeader,
  compileTileset,
  compileTilesetHeader,
  paletteSymbol,
  compilePalette,
  compilePaletteHeader,
  compileFont,
  compileFontHeader,
  compileFrameImage,
  compileFrameImageHeader,
  compileCursorImage,
  compileCursorImageHeader,
  compileScriptHeader,
  compileGameGlobalsInclude,
  compileAvatarFontHeader,
  compileAvatarFont,
  compileEmoteHeader,
  compileEmote,
  compileSceneProjectiles,
  compileSceneProjectilesHeader,
  compileSaveSignature,
  PrecompiledBackground,
  PrecompiledSprite,
  PrecompiledProjectile,
  ProjectileData,
  PrecompiledScene,
  PrecompiledPalette,
  PrecompiledSceneEventPtrs,
  sceneName,
} from "./generateGBVMData";
import compileSGBImage from "./sgb";
import { compileScriptEngineInit } from "./compileBootstrap";
import {
  compileMusicTracks,
  compileMusicHeader,
  compileFamiStudioSettings,
  PrecompiledMusicTrack,
} from "./compileMusic";
import { chunk } from "shared/lib/helpers/array";
import {
  ScriptBuilderEntity,
  ScriptBuilderEntityType,
  toProjectileHash,
} from "./scriptBuilder";
import {
  calculateAutoFadeEventId,
  isEmptyScript,
} from "shared/lib/scripts/eventHelpers";
import copy from "lib/helpers/fsCopy";
import { ensureDir } from "fs-extra";
import Path from "path";
import { determineUsedAssets } from "./precompile/determineUsedAssets";
import { compileSound } from "./sounds/compileSound";
import { readFileToTilesData } from "lib/tiles/readFileToTiles";
import l10n from "shared/lib/lang/l10n";
import type { ProjectData } from "store/features/project/projectActions";
import {
  AvatarData,
  BackgroundData,
  CustomEvent,
  EmoteData,
  FontData,
  MusicData,
  Palette,
  Scene,
  ScriptEvent,
  SpriteSheetData,
  TilesetData,
} from "shared/lib/entities/entitiesTypes";
import type { Dictionary } from "@reduxjs/toolkit";
import type {
  EngineFieldSchema,
  SceneTypeSchema,
} from "store/features/engine/engineState";
import type { Reference } from "components/forms/ReferencesSelect";
import type {
  ColorModeSetting,
  MusicDriverSetting,
  SettingsState,
} from "store/features/settings/settingsState";
import { ensureNumber, ensureString, ensureTypeGenerator } from "shared/types";
import { walkSceneScripts, walkScenesScripts } from "shared/lib/scripts/walk";
import { ScriptEventHandlers } from "lib/project/loadScriptEventHandlers";
import { EntityType } from "shared/lib/scripts/context";
import compileTilesets from "lib/compiler/compileTilesets";
import { DEFAULT_WHITE, DEFAULT_LIGHT, DEFAULT_DARK, DEFAULT_BLACK, defaultColors } from "consts";

type CompiledTilemapData = {
  symbol: string;
  data: number[] | Uint8Array;
  is360: boolean;
};

type CompiledTilesetData = {
  symbol: string;
  data: number[] | Uint8Array;
};

export type SceneMapData = {
  id: string;
  name: string;
  symbol: string;
};

export type VariableMapData = {
  id: string;
  name: string;
  symbol: string;
  isLocal: boolean;
  entityType: EntityType;
  entityId: string;
  sceneId: string;
};

const indexById = <T extends { id: string }>(arr: T[]) => keyBy(arr, "id");

const isReference = (value: unknown): value is Reference =>
  !!value &&
  typeof value === "object" &&
  typeof (value as Reference).id === "string" &&
  typeof (value as Reference).type === "string";

const isReferenceArray = (value: unknown): value is Reference[] => {
  if (!Array.isArray(value)) {
    return false;
  }
  return value.every(isReference);
};

const ensureReferenceArray = ensureTypeGenerator(isReferenceArray);

const isProjectileData = (value: unknown): value is ProjectileData => {
  return !!value && typeof value === "object";
};

export const EVENT_START_DATA_COMPILE = "EVENT_START_DATA_COMPILE";
export const EVENT_DATA_COMPILE_PROGRESS = "EVENT_DATA_COMPILE_PROGRESS";
export const EVENT_END_DATA_COMPILE = "EVENT_END_DATA_COMPILE";

export const EVENT_MSG_PRE_VARIABLES = "Preparing variables...";
export const EVENT_MSG_PRE_IMAGES = "Preparing images...";
export const EVENT_MSG_PRE_TILESETS = "Preparing tilesets...";
export const EVENT_MSG_PRE_UI_IMAGES = "Preparing ui...";
export const EVENT_MSG_PRE_SPRITES = "Preparing sprites...";
export const EVENT_MSG_PRE_AVATARS = "Preparing avatars...";
export const EVENT_MSG_PRE_EMOTES = "Preparing emotes...";
export const EVENT_MSG_PRE_SCENES = "Preparing scenes...";
export const EVENT_MSG_PRE_EVENTS = "Preparing events...";
export const EVENT_MSG_PRE_MUSIC = "Preparing music...";
export const EVENT_MSG_PRE_SOUNDS = "Preparing sounds...";
export const EVENT_MSG_PRE_FONTS = "Preparing fonts...";

export const EVENT_MSG_PRE_COMPLETE = "Preparation complete";
export const EVENT_MSG_COMPILING_EVENTS = "Compiling events...";

const ensureProjectAsset = async (
  relativePath: string,
  {
    projectRoot,
    warnings,
  }: {
    projectRoot: string;
    warnings: (msg: string) => void;
  }
) => {
  const projectPath = `${projectRoot}/${relativePath}`;
  const defaultPath = `${projectTemplatesRoot}/gbhtml/${relativePath}`;
  try {
    await ensureDir(Path.dirname(projectPath));
    await copy(defaultPath, projectPath, {
      overwrite: false,
      errorOnExist: true,
    });
    warnings &&
      warnings(
        `${relativePath} was missing, copying default file to project assets`
      );
  } catch (e) {
    // Don't need to catch this, if it failed then the file already exists
    // and we can safely continue.
  }
  return `${projectPath}`;
};


function toHexString(byteArray: Uint8Array) {
  return Array.from(byteArray, function(byte) {
    return ('0' + (byte & 0xFF).toString(16)).slice(-2);
  }).join(' ')
}


// #region precompile

export const precompileBackgrounds = async (
  backgrounds: BackgroundData[],
  scenes: Scene[],
  tilesets: TilesetData[],
  customEventsLookup: Dictionary<CustomEvent>,
  colorMode: ColorModeSetting,
  projectRoot: string,
  tmpPath: string,
  {
    warnings,
  }: {
    warnings: (_msg: string) => void;
  }
) => {
  const usedTilemaps: CompiledTilemapData[] = [];
  const usedTilemapAttrs: CompiledTilemapData[] = [];
  const usedTilemapAttrsNES: CompiledTilemapData[] = [];

  const eventImageIds: string[] = [];
  walkScenesScripts(
    scenes,
    {
      customEvents: {
        lookup: customEventsLookup,
        maxDepth: MAX_NESTED_SCRIPT_DEPTH,
      },
    },
    (cmd) => {
      if (eventHasArg(cmd, "backgroundId")) {
        eventImageIds.push(ensureString(cmd.args?.backgroundId, ""));
      } else if (eventHasArg(cmd, "references")) {
        const referencedIds = ensureReferenceArray(cmd.args?.references, [])
          .filter((ref) => ref.type === "background")
          .map((ref) => ref.id);
        eventImageIds.push(...referencedIds);
      }
    }
  );
  const usedBackgrounds = backgrounds.filter(
    (background) =>
      eventImageIds.indexOf(background.id) > -1 ||
      scenes.find((scene) => scene.backgroundId === background.id)
  );

  const tilesetLookup = keyBy(tilesets, "id");

  const commonTilesetsLookup = scenes.reduce((memo, scene) => {
    if (!scene.backgroundId || !scene.tilesetId) {
      return memo;
    }
    const tileset = tilesetLookup[scene.tilesetId];
    if (memo[scene.backgroundId]) {
      if (!memo[scene.backgroundId].find((t) => t.id === scene.tilesetId)) {
        memo[scene.backgroundId].push(tileset);
      }
    } else {
      memo[scene.backgroundId] = [tileset];
    }
    return memo;
  }, {} as Record<string, TilesetData[]>);

  // List of ids to generate 360 tiles
  const generate360Ids = usedBackgrounds
    .filter((background) =>
      scenes.find(
        (scene) => scene.backgroundId === background.id && scene.type === "LOGO"
      )
    )
    .map((background) => background.id);

  const backgroundsData = await compileImages(
    usedBackgrounds,
    commonTilesetsLookup,
    generate360Ids,
    colorMode,
    projectRoot,
    {
      warnings,
    }
  );

  const usedTilesets: CompiledTilesetData[] = [];

  const usedBackgroundsWithData: PrecompiledBackground[] = backgroundsData.map(
    (background) => {
      // Determine tileset
      let tileset1Index = -1;
      let tileset2Index = -1;
      let tilemapIndex = -1;
      let tilemapAttrIndex = -1;
      let tilemapAttrIndexNES = -1;
      let attr_nes_width = 0;
      let attr_nes_height = 0;

      // VRAM Bank 1
      if (background.vramData[0].length > 0) {
        tileset1Index = usedTilesets.length;
        usedTilesets.push({
          symbol: `${background.symbol}_tileset`,
          data: background.vramData[0],
        });
      }

      // VRAM Bank 2
      if (background.vramData[1].length > 0) {
        tileset2Index = usedTilesets.length;
        usedTilesets.push({
          symbol: `${background.symbol}_cgb_tileset`,
          data: background.vramData[1],
        });
      }

      // Extract Tilemap
      if (background.tilemap.length > 0) {
        tilemapIndex = usedTilemaps.length;
        usedTilemaps.push({
          symbol: `${background.symbol}_tilemap`,
          data: background.tilemap,
          is360: generate360Ids.includes(background.id),
        });
      }

      // Extract Tilemap Attr
      if (background.attr.length > 0) {
        tilemapAttrIndex = usedTilemapAttrs.length;
        usedTilemapAttrs.push({
          symbol: `${background.symbol}_tilemap_attr`,
          data: background.attr,
          is360: generate360Ids.includes(background.id),
        });
      }

// export interface PrecompiledTileData {
//   symbol: string;
//   data: number[] | Uint8Array;
// }

      // Reduce Tilemap Attrs to NES 16x16 format
      if (background.attr.length > 0) {
      // usedTilemapAttrsNES.length
        var tilemapAttrNES_map = convertAttrsFormatNES(usedTilemapAttrs[usedTilemapAttrsNES.length].data, background.width, background.height);
        attr_nes_width = tilemapAttrNES_map.width;
        attr_nes_height = tilemapAttrNES_map.height;
        //console.log(`${background.symbol}_tilemap_attr_nes = ${tilemapAttrNES_map}, background.width = ${background.width}, background.height = ${background.height}`);
        console.log(`${background.symbol}_tilemap_attr: (${background.width} x ${background.height})`)
        for(var y = 0; y < background.height; y++)
        {
            var attrLine = usedTilemapAttrs[usedTilemapAttrsNES.length].data.slice(y * background.width, (y + 1) * background.width);
            console.log(`  ${attrLine}`);
        }
        console.log(`${background.symbol}_tilemap_attr_nes: (${tilemapAttrNES_map.width} x ${tilemapAttrNES_map.height})`)
        for(var y = 0; y < 4*tilemapAttrNES_map.height; y++)
        {
            var attrLineNES = toHexString(tilemapAttrNES_map.data.slice(y * tilemapAttrNES_map.width, (y + 1) * tilemapAttrNES_map.width));
            console.log(`  ${attrLineNES}`);
        }
        tilemapAttrIndexNES = usedTilemapAttrsNES.length;
        usedTilemapAttrsNES.push({
          symbol: `${background.symbol}_tilemap_attr_nes`,
          data: new Uint8Array(tilemapAttrNES_map.data),
          is360: false,
        });
      }

      return {
        ...background,
        tileset: usedTilesets[tileset1Index],
        cgbTileset: usedTilesets[tileset2Index],
        tilemap: usedTilemaps[tilemapIndex],
        tilemapAttr: usedTilemapAttrs[tilemapAttrIndex],
        tilemapAttrNES: usedTilemapAttrsNES[tilemapAttrIndexNES],
        tilemapAttrNES_width: attr_nes_width,
        tilemapAttrNES_height: attr_nes_height
      };
    }
  );

  const backgroundLookup = indexById(usedBackgroundsWithData);

  return {
    usedBackgrounds: usedBackgroundsWithData,
    usedTilesets,
    backgroundLookup,
    usedTilemaps,
    usedTilemapAttrs,
    usedTilemapAttrsNES,
  };
};

export const precompilePalettes = async (
  scenes: Scene[],
  settings: SettingsState,
  palettes: Palette[],
  backgrounds: Dictionary<PrecompiledBackground>
) => {
  const usedPalettes: PrecompiledPalette[] = [];
  const usedPalettesCache: Record<string, number> = {};
  const scenePaletteIndexes: Record<string, number> = {};
  const sceneActorPaletteIndexes: Record<string, number> = {};
  const actorPaletteIndexes = {};

  const isColor = settings.colorMode !== "mono" || settings.sgbEnabled;

  const palettesLookup = indexById(palettes);
  const defaultBackgroundPaletteIds =
    settings.defaultBackgroundPaletteIds || [];
  const defaultSpritePaletteIds = settings.defaultSpritePaletteIds || [];

  const getPalette = (id: string, fallbackId: string): Palette => {
    if (id === "dmg") {
      return DMG_PALETTE;
    }
    return palettesLookup[id] || palettesLookup[fallbackId] || DMG_PALETTE;
  };

  const getBackgroundPalette = (
    index: number,
    sceneBackgroundPaletteIds: string[],
    defaultBackgroundPaletteIds: string[],
    autoPalettes?: Palette[]
  ) => {
    if (autoPalettes?.[index]) {
      return autoPalettes[index];
    }
    return getPalette(
      sceneBackgroundPaletteIds[index],
      defaultBackgroundPaletteIds[index]
    );
  };

  const getSpritePalette = (id: string, fallbackId: string): Palette => {
    const p = getPalette(id, fallbackId);
    const nesColors: [string, string, string, string] = p?.nesColors || defaultColors;
    return {
      ...p,
      colors: [nesColors[0], nesColors[0], nesColors[1], nesColors[3]],
    };
  };

  // Background palettes

  for (let i = 0; i < scenes.length; i++) {
    const scene = scenes[i];
    const sceneBackgroundPaletteIds = scene.paletteIds || [];

    const background = backgrounds[scene.backgroundId];
    if (background?.autoPalettes?.[0]) {
    }

    const scenePalette = {
      dmg: [
        ["DMG_WHITE", "DMG_LITE_GRAY", "DMG_DARK_GRAY", "DMG_BLACK"] as [
          string,
          string,
          string,
          string
        ],
      ],
      colors: isColor
        ? [
            getBackgroundPalette(
              0,
              sceneBackgroundPaletteIds,
              defaultBackgroundPaletteIds,
              background?.autoPalettes
            ),
            getBackgroundPalette(
              1,
              sceneBackgroundPaletteIds,
              defaultBackgroundPaletteIds,
              background?.autoPalettes
            ),
            getBackgroundPalette(
              2,
              sceneBackgroundPaletteIds,
              defaultBackgroundPaletteIds,
              background?.autoPalettes
            ),
            getBackgroundPalette(
              3,
              sceneBackgroundPaletteIds,
              defaultBackgroundPaletteIds,
              background?.autoPalettes
            ),
            getBackgroundPalette(
              4,
              sceneBackgroundPaletteIds,
              defaultBackgroundPaletteIds,
              background?.autoPalettes
            ),
            getBackgroundPalette(
              5,
              sceneBackgroundPaletteIds,
              defaultBackgroundPaletteIds,
              background?.autoPalettes
            ),
            getBackgroundPalette(
              6,
              sceneBackgroundPaletteIds,
              defaultBackgroundPaletteIds,
              background?.autoPalettes
            ),
            getBackgroundPalette(
              7,
              sceneBackgroundPaletteIds,
              defaultBackgroundPaletteIds,
              background?.autoPalettes
            ),
        ].map((p) => p?.nesColors || defaultColors as [string, string, string, string])
        : undefined,
    };

    const scenePaletteKey = JSON.stringify(scenePalette);
    if (usedPalettesCache[scenePaletteKey] === undefined) {
      // New palette
      const paletteIndex = usedPalettes.length;
      usedPalettes.push(scenePalette);
      usedPalettesCache[scenePaletteKey] = paletteIndex;
      scenePaletteIndexes[scene.id] = paletteIndex;
    } else {
      // Already used palette
      scenePaletteIndexes[scene.id] = usedPalettesCache[scenePaletteKey];
    }
  }

  // Actor palettes

  for (let i = 0; i < scenes.length; i++) {
    const scene = scenes[i];
    const sceneSpritePaletteIds = scene.spritePaletteIds || [];

    const actorsPalette = {
      dmg: [
        ["DMG_WHITE", "DMG_WHITE", "DMG_LITE_GRAY", "DMG_BLACK"] as [
          string,
          string,
          string,
          string
        ],
        ["DMG_WHITE", "DMG_WHITE", "DMG_DARK_GRAY", "DMG_BLACK"] as [
          string,
          string,
          string,
          string
        ],
      ],
      colors: isColor
        ? [
            getSpritePalette(
              sceneSpritePaletteIds[0],
              defaultSpritePaletteIds[0]
            ),
            getSpritePalette(
              sceneSpritePaletteIds[1],
              defaultSpritePaletteIds[1]
            ),
            getSpritePalette(
              sceneSpritePaletteIds[2],
              defaultSpritePaletteIds[2]
            ),
            getSpritePalette(
              sceneSpritePaletteIds[3],
              defaultSpritePaletteIds[3]
            ),
            getSpritePalette(
              sceneSpritePaletteIds[4],
              defaultSpritePaletteIds[4]
            ),
            getSpritePalette(
              sceneSpritePaletteIds[5],
              defaultSpritePaletteIds[5]
            ),
            getSpritePalette(
              sceneSpritePaletteIds[6],
              defaultSpritePaletteIds[6]
            ),
            getSpritePalette(
              sceneSpritePaletteIds[7],
              defaultSpritePaletteIds[7]
            ),
          ].map((p) => p?.colors || defaultColors as [string, string, string, string])
          : undefined,
    };
    const actorsPaletteKey = JSON.stringify(actorsPalette);
    if (usedPalettesCache[actorsPaletteKey] === undefined) {
      // New palette
      const paletteIndex = usedPalettes.length;
      usedPalettes.push(actorsPalette);
      usedPalettesCache[actorsPaletteKey] = paletteIndex;
      sceneActorPaletteIndexes[scene.id] = paletteIndex;
    } else {
      // Already used palette
      sceneActorPaletteIndexes[scene.id] = usedPalettesCache[actorsPaletteKey];
    }
  }

  return {
    usedPalettes,
    scenePaletteIndexes,
    sceneActorPaletteIndexes,
    actorPaletteIndexes,
  };
};

export const precompileUIImages = async (
  projectRoot: string,
  tmpPath: string,
  {
    warnings,
  }: {
    warnings: (_msg: string) => void;
  }
) => {
  const framePath = await ensureProjectAsset("assets/ui/frame.png", {
    projectRoot,
    warnings,
  });
  const cursorPath = await ensureProjectAsset("assets/ui/cursor.png", {
    projectRoot,
    warnings,
  });

  const frameTiles = await readFileToTilesData(framePath);
  const cursorTiles = await readFileToTilesData(cursorPath);

  return { frameTiles, cursorTiles };
};

export const precompileSprites = async (
  spriteSheets: SpriteSheetData[],
  scenes: Scene[],
  customEventsLookup: Dictionary<CustomEvent>,
  defaultPlayerSprites: Record<string, string>,
  cgbOnly: boolean,
  projectRoot: string
) => {
  const usedSprites: SpriteSheetData[] = [];
  const usedTilesets: CompiledTilesetData[] = [];

  const usedSpriteLookup: Record<string, SpriteSheetData> = {};
  const spriteLookup = indexById(spriteSheets);

  const addSprite = (spriteSheetId: string) => {
    if (!usedSpriteLookup[spriteSheetId] && spriteLookup[spriteSheetId]) {
      const spriteSheet = spriteLookup[spriteSheetId];
      usedSprites.push(spriteSheet);
      usedSpriteLookup[spriteSheetId] = spriteSheet;
    }
  };

  walkScenesScripts(
    scenes,
    {
      customEvents: {
        lookup: customEventsLookup,
        maxDepth: MAX_NESTED_SCRIPT_DEPTH,
      },
    },
    (cmd) => {
      if (cmd.args) {
        if (cmd.args.spriteSheetId) {
          addSprite(ensureString(cmd.args.spriteSheetId, ""));
        }
      }
      if (eventHasArg(cmd, "references")) {
        const referencedIds = ensureReferenceArray(cmd.args?.references, [])
          .filter((ref) => ref.type === "sprite")
          .map((ref) => ref.id);
        for (const id of referencedIds) {
          addSprite(id);
        }
      }
    }
  );

  for (let i = 0; i < scenes.length; i++) {
    const scene = scenes[i];
    addSprite(ensureString(scene.playerSpriteSheetId, ""));
    addSprite(defaultPlayerSprites[scene.type]);
    for (let a = 0; a < scene.actors.length; a++) {
      const actor = scene.actors[a];
      addSprite(actor.spriteSheetId);
    }
  }

  const { spritesData, statesOrder, stateReferences } = await compileSprites(
    usedSprites,
    cgbOnly,
    projectRoot
  );

  const usedSpritesWithData: PrecompiledSprite[] = spritesData.map((sprite) => {
    // Determine tileset
    let tileset1Index = -1;
    let tileset2Index = -1;

    // VRAM Bank 1
    if (sprite.vramData[0].length > 0) {
      tileset1Index = usedTilesets.length;
      usedTilesets.push({
        symbol: `${sprite.symbol}_tileset`,
        data: sprite.vramData[0],
      });
    }

    // VRAM Bank 2
    if (sprite.vramData[1].length > 0) {
      tileset2Index = usedTilesets.length;
      usedTilesets.push({
        symbol: `${sprite.symbol}_bank2_tileset`,
        data: sprite.vramData[1],
      });
    }

    return {
      ...sprite,
      tileset: usedTilesets[tileset1Index],
      cgbTileset: usedTilesets[tileset2Index],
    };
  });

  return {
    usedSprites: usedSpritesWithData,
    usedTilesets,
    statesOrder,
    stateReferences,
    spriteLookup,
  };
};

export const precompileAvatars = async (
  avatars: AvatarData[],
  scenes: Scene[],
  customEventsLookup: Dictionary<CustomEvent>,
  projectRoot: string,
  {
    warnings,
  }: {
    warnings: (msg: string) => void;
  }
) => {
  const usedAvatars: AvatarData[] = [];
  const usedAvatarLookup: Record<string, AvatarData> = {};
  const avatarLookup = indexById(avatars);

  walkScenesScripts(
    scenes,
    {
      customEvents: {
        lookup: customEventsLookup,
        maxDepth: MAX_NESTED_SCRIPT_DEPTH,
      },
    },
    (event) => {
      if (event.args) {
        const avatarId = ensureString(event.args.avatarId, "");
        if (avatarId && !usedAvatarLookup[avatarId] && avatarLookup[avatarId]) {
          const avatar = avatarLookup[avatarId];
          usedAvatars.push(avatar);
          usedAvatarLookup[avatarId] = avatar;
        }
      }
    }
  );

  const avatarData = await compileAvatars(usedAvatars, projectRoot, {
    warnings,
  });

  return {
    usedAvatars: avatarData,
    avatarLookup,
  };
};

export const precompileEmotes = async (
  emotes: EmoteData[],
  scenes: Scene[],
  customEventsLookup: Dictionary<CustomEvent>,
  projectRoot: string,
  {
    warnings,
  }: {
    warnings: (msg: string) => void;
  }
) => {
  const usedEmotes: EmoteData[] = [];
  const usedEmoteLookup: Record<string, EmoteData> = {};
  const emoteLookup = indexById(emotes);

  const addEmote = (id: string) => {
    const emote = emoteLookup[id];
    if (!usedEmoteLookup[id] && emote) {
      usedEmotes.push(emote);
      usedEmoteLookup[id] = emote;
    }
  };

  walkScenesScripts(
    scenes,
    {
      customEvents: {
        lookup: customEventsLookup,
        maxDepth: MAX_NESTED_SCRIPT_DEPTH,
      },
    },
    (cmd) => {
      if (cmd.args && cmd.args.emoteId) {
        addEmote(ensureString(cmd.args.emoteId, ""));
      }
      if (eventHasArg(cmd, "references")) {
        const referencedIds = ensureReferenceArray(cmd.args?.references, [])
          .filter((ref) => ref.type === "emote")
          .map((ref) => ref.id);
        for (const id of referencedIds) {
          addEmote(id);
        }
      }
    }
  );

  const emoteData = await compileEmotes(usedEmotes, projectRoot, {
    warnings,
  });

  return {
    usedEmotes: emoteData,
    emoteLookup,
  };
};

export const precompileTilesets = async (
  tilesets: TilesetData[],
  scenes: Scene[],
  customEventsLookup: Dictionary<CustomEvent>,
  projectRoot: string,
  {
    warnings,
  }: {
    warnings: (msg: string) => void;
  }
) => {
  const usedTilesets: TilesetData[] = [];
  const usedTilesetLookup: Record<string, TilesetData> = {};
  const tilesetLookup = indexById(tilesets);

  const addTileset = (id: string) => {
    const tileset = tilesetLookup[id];
    if (!usedTilesetLookup[id] && tileset) {
      usedTilesets.push(tileset);
      usedTilesetLookup[id] = tileset;
    }
  };

  walkScenesScripts(
    scenes,
    {
      customEvents: {
        lookup: customEventsLookup,
        maxDepth: MAX_NESTED_SCRIPT_DEPTH,
      },
    },
    (cmd) => {
      if (cmd.args && cmd.args.tilesetId) {
        addTileset(ensureString(cmd.args.tilesetId, ""));
      }
      if (eventHasArg(cmd, "references")) {
        const referencedIds = ensureReferenceArray(cmd.args?.references, [])
          .filter((ref) => ref.type === "tileset")
          .map((ref) => ref.id);
        for (const id of referencedIds) {
          addTileset(id);
        }
      }
    }
  );

  const tilesetData = await compileTilesets(usedTilesets, projectRoot, {
    warnings,
  });

  return {
    usedTilesets: tilesetData,
    tilesetLookup,
  };
};

export const precompileMusic = (
  scenes: Scene[],
  customEventsLookup: Dictionary<CustomEvent>,
  music: MusicData[],
  musicDriver: MusicDriverSetting
) => {
  const usedMusicIds: string[] = [];
  const driverMusic =
    musicDriver === "huge"
      ? music.filter((track) => track.type === "uge")
      : music.filter((track) => track.type !== "uge");

  walkScenesScripts(
    scenes,
    {
      customEvents: {
        lookup: customEventsLookup,
        maxDepth: MAX_NESTED_SCRIPT_DEPTH,
      },
    },
    (cmd) => {
      if (
        cmd.args &&
        (cmd.args.musicId !== undefined || cmd.command === EVENT_MUSIC_PLAY)
      ) {
        console.log(`precompileMusic: Found EVENT_MUSIC_PLAY command: cmd.args.musicId = ${cmd.args.musicId}, music[0].id = ${music[0].id}`); // DEBUGHACK
        const musicId = ensureString(cmd.args.musicId, music[0].id);
        // If never seen this track before add it to the list
        if (usedMusicIds.indexOf(musicId) === -1) {
          usedMusicIds.push(musicId);
        }
      } else if (eventHasArg(cmd, "references")) {
        const referencedIds = ensureReferenceArray(cmd.args?.references, [])
          .filter((ref) => ref.type === "music")
          .map((ref) => ref.id);
        console.log(`precompileMusic: Found references: referencedIds = ${referencedIds}`); // DEBUGHACK
        usedMusicIds.push(...referencedIds);
      }
    }
  );

  const usedMusic: PrecompiledMusicTrack[] = music
    .filter((track) => {
      return usedMusicIds.indexOf(track.id) > -1;
    })
    .map((track) => {
      // If wrong driver needed, fallback to first driver track
      if (
        (musicDriver === "huge" && track.type === "uge") ||
        (musicDriver !== "huge" && track.type !== "uge")
      ) {
        console.log(`usedMusic: return track = ${track}`); // DEBUGHACK
        return track;
      }
      console.log(`usedMusic: return track = ..driverMusic = ${driverMusic[0]}, id = ${track.id}`); // DEBUGHACK
      return {
        ...driverMusic[0],
        id: track.id,
      };
    })
    .filter((track) => track)
    .map((track) => {
      return {
        ...track,
        dataName: track.symbol,
      };
    });
  return { usedMusic };
};

export const precompileFonts = async (
  usedFonts: FontData[],
  scenes: Scene[],
  customEventsLookup: Dictionary<CustomEvent>,
  defaultFontId: string,
  projectRoot: string,
  {
    warnings,
  }: {
    warnings: (msg: string) => void;
  }
) => {
  if (usedFonts.length === 0) {
    await ensureProjectAsset("assets/fonts/gbs-mono.png", {
      projectRoot,
      warnings,
    });
    await ensureProjectAsset("assets/fonts/gbs-mono.json", {
      projectRoot,
      warnings,
    });
    throw new Error(l10n("ERROR_MISSING_FONTS"));
  }

  const fontData = await compileFonts(usedFonts, projectRoot);

  return { usedFonts: fontData };
};

export const precompileScenes = (
  scenes: Scene[],
  customEventsLookup: Dictionary<CustomEvent>,
  defaultPlayerSprites: Record<string, string>,
  colorMode: ColorModeSetting,
  usedBackgrounds: PrecompiledBackground[],
  usedSprites: PrecompiledSprite[],
  {
    warnings,
  }: {
    warnings: (msg: string) => void;
  }
) => {
  const scenesData: PrecompiledScene[] = scenes.map((scene, sceneIndex) => {
    const background = usedBackgrounds.find(
      (background) =>
        background.id === scene.backgroundId &&
        (!scene.tilesetId || background.commonTilesetId === scene.tilesetId)
    );
    if (!background) {
      throw new Error(
        `Scene #${sceneIndex + 1} ${
          scene.name ? `'${scene.name}'` : ""
        } has missing or no background assigned.`
      );
    }

    const usedSpritesLookup = keyBy(usedSprites, "id");

    if (scene.actors.length > MAX_ACTORS) {
      warnings(
        `Scene #${sceneIndex + 1} ${
          scene.name ? `'${scene.name}'` : ""
        } contains ${
          scene.actors.length
        } actors when maximum is ${MAX_ACTORS}. Some actors will be removed.`
      );
    }

    if (scene.triggers.length > MAX_TRIGGERS) {
      warnings(
        `Scene #${sceneIndex + 1} ${
          scene.name ? `'${scene.name}'` : ""
        } contains ${
          scene.triggers.length
        } triggers when maximum is ${MAX_TRIGGERS}. Some triggers will be removed.`
      );
    }

    const actors = scene.actors.slice(0, MAX_ACTORS).filter((actor) => {
      return usedSprites.find((s) => s.id === actor.spriteSheetId);
    });

    const eventSpriteIds: string[] = [];
    const playerSpriteSheetId = scene.playerSpriteSheetId
      ? scene.playerSpriteSheetId
      : defaultPlayerSprites[scene.type];

    let playerSprite = usedSprites.find((s) => s.id === playerSpriteSheetId);

    if (!playerSprite && scene.type !== "LOGO") {
      warnings(
        l10n("WARNING_NO_PLAYER_SET_FOR_SCENE_TYPE", { type: scene.type })
      );
      playerSprite = usedSprites[0];
    }

    const projectiles: PrecompiledProjectile[] = [];
    const actorsExclusiveLookup: Record<string, number> = {};
    const addProjectile = (data: ProjectileData) => {
      const projectile = {
        ...data,
        hash: toProjectileHash({
          spriteSheetId: data.spriteSheetId,
          spriteStateId: data.spriteStateId,
          speed: data.speed,
          animSpeed: data.animSpeed,
          lifeTime: data.lifeTime,
          initialOffset: data.initialOffset,
          collisionGroup: data.collisionGroup,
          collisionMask: data.collisionMask,
        }),
      };
      if (!projectiles.find((p) => p.hash === projectile.hash)) {
        projectiles.push(projectile);
      }
    };

    const getSpriteTileCount = (sprite: PrecompiledSprite | undefined) => {
      const count = ((sprite ? sprite.numTiles : 0) || 0) * 2;
      if (colorMode === "color") {
        return Math.ceil(count / 4) * 2;
      }
      return count;
    };

    walkSceneScripts(
      scene,
      {
        customEvents: {
          lookup: customEventsLookup,
          maxDepth: MAX_NESTED_SCRIPT_DEPTH,
        },
      },
      (event, actor, _trigger) => {
        if (
          event.args &&
          event.args.spriteSheetId &&
          event.command !== EVENT_PLAYER_SET_SPRITE &&
          event.command !== EVENT_ACTOR_SET_SPRITE &&
          !event.args.__comment
        ) {
          eventSpriteIds.push(ensureString(event.args.spriteSheetId, ""));
        }

        if (
          event.args &&
          event.args.spriteSheetId &&
          event.command === "EVENT_LAUNCH_PROJECTILE" &&
          !event.args.__comment &&
          isProjectileData(event.args)
        ) {
          addProjectile(event.args);
        }

        if (
          event.args &&
          event.args.spriteSheetId &&
          event.command === EVENT_ACTOR_SET_SPRITE
        ) {
          let actorId = ensureString(event.args.actorId, "");
          if (actorId === "$self$") {
            if (actor) {
              actorId = actor.id;
            } else {
              actorId = "player";
            }
          }
          const sprite =
            usedSpritesLookup[ensureString(event.args.spriteSheetId, "")];
          actorsExclusiveLookup[actorId] = Math.max(
            actorsExclusiveLookup[actorId] || 0,
            getSpriteTileCount(sprite)
          );
        }

        if (
          event.args &&
          event.args.spriteSheetId &&
          event.command === EVENT_PLAYER_SET_SPRITE
        ) {
          const actorId = "player";
          const sprite =
            usedSpritesLookup[ensureString(event.args.spriteSheetId, "")];
          actorsExclusiveLookup[actorId] = Math.max(
            actorsExclusiveLookup[actorId] || 0,
            getSpriteTileCount(sprite)
          );
        }
      }
    );

    const actorSpriteIds = actors
      .filter((a) => !actorsExclusiveLookup[a.id])
      .map((a) => a.spriteSheetId);

    const sceneSpriteIds = ([] as string[]).concat(
      actorSpriteIds,
      eventSpriteIds
    );

    if (projectiles.length > MAX_PROJECTILES) {
      warnings(
        l10n("WARNING_TOO_MANY_UNIQUE_PROJECTILES", {
          name: scene.name,
          num: projectiles.length,
          max: MAX_PROJECTILES,
        })
      );
      projectiles.splice(MAX_PROJECTILES);
    }

    return {
      ...scene,
      playerSpriteSheetId: playerSprite ? playerSprite.id : undefined,
      background,
      actors,
      sprites: sceneSpriteIds.reduce((memo, spriteId) => {
        const sprite = usedSprites.find((s) => s.id === spriteId);
        if (sprite && memo.indexOf(sprite) === -1) {
          memo.push(sprite);
        }
        return memo;
      }, [] as PrecompiledSprite[]),
      triggers: scene.triggers.slice(0, MAX_TRIGGERS).filter((trigger) => {
        // Filter out unused triggers which cause slow down
        // When walking over
        return (
          (trigger.script &&
            trigger.script.length >= 1 &&
            trigger.script[0].command !== EVENT_END) ||
          (trigger.leaveScript &&
            trigger.leaveScript.length >= 1 &&
            trigger.leaveScript[0].command !== EVENT_END)
        );
      }),
      playerSprite,
      actorsExclusiveLookup,
      actorsData: [],
      triggersData: [],
      projectiles,
    };
  });
  return scenesData;
};

const precompile = async (
  projectData: ProjectData,
  projectRoot: string,
  scriptEventHandlers: ScriptEventHandlers,
  tmpPath: string,
  {
    progress,
    warnings,
  }: {
    progress: (msg: string) => void;
    warnings: (msg: string) => void;
  }
) => {
  const customEventsLookup = keyBy(projectData.customEvents, "id");
  const colorMode = projectData.settings.colorMode;
  const cgbOnly = colorMode === "color";

  const usedAssets = determineUsedAssets({
    projectData,
    customEventsLookup,
    scriptEventHandlers,
  });

  progress(EVENT_MSG_PRE_VARIABLES);
  const usedVariables = usedAssets.referencedVariables;

  progress(EVENT_MSG_PRE_IMAGES);
  const {
    usedBackgrounds,
    backgroundLookup,
    usedTilesets: usedBackgroundTilesets,
    usedTilemaps,
    usedTilemapAttrs,
    usedTilemapAttrsNES,
  } = await precompileBackgrounds(
    projectData.backgrounds,
    projectData.scenes,
    projectData.tilesets,
    customEventsLookup,
    colorMode,
    projectRoot,
    tmpPath,
    { warnings }
  );

  progress(EVENT_MSG_PRE_TILESETS);
  const { usedTilesets } = await precompileTilesets(
    projectData.tilesets,
    projectData.scenes,
    customEventsLookup,
    projectRoot,
    { warnings }
  );

  progress(EVENT_MSG_PRE_UI_IMAGES);
  const { frameTiles, cursorTiles } = await precompileUIImages(
    projectRoot,
    tmpPath,
    {
      warnings,
    }
  );

  progress(EVENT_MSG_PRE_SPRITES);
  const {
    usedSprites,
    usedTilesets: usedSpriteTilesets,
    statesOrder,
    stateReferences,
  } = await precompileSprites(
    projectData.spriteSheets,
    projectData.scenes,
    customEventsLookup,
    projectData.settings.defaultPlayerSprites,
    cgbOnly,
    projectRoot
  );

  progress(EVENT_MSG_PRE_AVATARS);
  const { usedAvatars } = await precompileAvatars(
    projectData.avatars || [],
    projectData.scenes,
    customEventsLookup,
    projectRoot,
    {
      warnings,
    }
  );

  progress(EVENT_MSG_PRE_EMOTES);
  const { usedEmotes } = await precompileEmotes(
    projectData.emotes || [],
    projectData.scenes,
    customEventsLookup,
    projectRoot,
    {
      warnings,
    }
  );

  progress(EVENT_MSG_PRE_MUSIC);
  const { usedMusic } = await precompileMusic(
    projectData.scenes,
    customEventsLookup,
    projectData.music,
    projectData.settings.musicDriver
  );

  progress(EVENT_MSG_PRE_FONTS);
  const { usedFonts } = await precompileFonts(
    usedAssets.referencedFonts,
    projectData.scenes,
    customEventsLookup,
    projectData.settings.defaultFontId,
    projectRoot,
    {
      warnings,
    }
  );

  progress(EVENT_MSG_PRE_SCENES);
  const sceneData = precompileScenes(
    projectData.scenes,
    customEventsLookup,
    projectData.settings.defaultPlayerSprites,
    colorMode,
    usedBackgrounds,
    usedSprites,
    {
      warnings,
    }
  );

  const {
    usedPalettes,
    scenePaletteIndexes,
    sceneActorPaletteIndexes,
    actorPaletteIndexes,
  } = await precompilePalettes(
    projectData.scenes,
    projectData.settings,
    projectData.palettes,
    backgroundLookup
  );

  const usedSounds = usedAssets.referencedSounds;

  progress(EVENT_MSG_PRE_COMPLETE);

  return {
    usedVariables,
    usedBackgrounds,
    backgroundLookup,
    usedTilesets,
    usedBackgroundTilesets,
    usedSpriteTilesets,
    usedTilemaps,
    usedTilemapAttrs,
    usedTilemapAttrsNES,
    usedSprites,
    statesOrder,
    stateReferences,
    usedMusic,
    usedSounds,
    usedFonts,
    sceneData,
    frameTiles,
    cursorTiles,
    usedAvatars,
    usedEmotes,
    usedPalettes,
    scenePaletteIndexes,
    sceneActorPaletteIndexes,
    actorPaletteIndexes,
  };
};

// #endregion

const compile = async (
  projectData: ProjectData,
  {
    projectRoot = "/tmp",
    scriptEventHandlers,
    engineFields = [],
    sceneTypes = [],
    tmpPath = "/tmp",
    debugEnabled = false,
    progress = (_msg: string) => {},
    warnings = (_msg: string) => {},
  }: {
    projectRoot: string;
    scriptEventHandlers: ScriptEventHandlers;
    engineFields: EngineFieldSchema[];
    sceneTypes: SceneTypeSchema[];
    tmpPath: string;
    debugEnabled?: boolean;
    progress: (_msg: string) => void;
    warnings: (_msg: string) => void;
  }
): Promise<{
  files: Record<string, string>;
  sceneMap: Record<string, SceneMapData>;
  variableMap: Record<string, VariableMapData>;
}> => {
  const output: Record<string, string> = {};
  const symbols: Dictionary<string> = {};
  const sceneMap: Record<string, SceneMapData> = {};

  if (projectData.scenes.length === 0) {
    throw new Error(
      "No scenes are included in your project. Add some scenes in the Game World editor and try again."
    );
  }

  const precompiled = await precompile(
    projectData,
    projectRoot,
    scriptEventHandlers,
    tmpPath,
    {
      progress,
      warnings,
    }
  );

  const colorEnabled = projectData.settings.colorMode !== "mono";
  const isCGBOnly = projectData.settings.colorMode === "color";
  const isSGB = projectData.settings.sgbEnabled && !isCGBOnly;
  const precompiledEngineFields = keyBy(engineFields, "key");
  const customEventsLookup = keyBy(projectData.customEvents, "id");

  // Add UI data
  output["frame_image.c"] = compileFrameImage(precompiled.frameTiles);
  output["frame_image.h"] = compileFrameImageHeader(precompiled.frameTiles);
  output["cursor_image.c"] = compileCursorImage(precompiled.cursorTiles);
  output["cursor_image.h"] = compileCursorImageHeader(precompiled.cursorTiles);

  if (isSGB) {
    const sgbPath = await ensureProjectAsset("assets/sgb/border.png", {
      projectRoot,
      warnings,
    });
    output["border.c"] = await compileSGBImage(sgbPath);
  }

  progress(EVENT_MSG_COMPILING_EVENTS);
  // Hacky small wait to allow console to update before event loop is blocked
  // Can maybe move some of the compilation into workers to prevent this
  await new Promise((resolve) => setTimeout(resolve, 20));

  const variablesLookup = keyBy(projectData.variables, "id");
  const variableAliasLookup = precompiled.usedVariables.reduce(
    (memo, variable) => {
      // Include variables referenced from GBVM
      if (variable.symbol) {
        const symbol = variable.symbol.toUpperCase();
        memo[variable.id] = {
          symbol,
          id: variable.id,
          name: variable.name,
          isLocal: false,
          entityType: "scene",
          entityId: "",
          sceneId: "",
        };
      }
      return memo;
    },
    {} as Record<string, VariableMapData>
  );

  // Add event data
  const additionalScripts: Dictionary<{
    symbol: string;
    sceneId: string;
    entityId: string;
    entityType: ScriptBuilderEntityType;
    scriptKey: string;
    compiledScript: string;
  }> = {};
  const additionalOutput: Dictionary<{
    filename: string;
    data: string;
  }> = {};
  const compiledCustomEventScriptCache: Dictionary<{
    scriptRef: string;
    argsLen: number;
  }> = {};
  const compiledAssetsCache: Dictionary<string> = {};

  const eventPtrs: PrecompiledSceneEventPtrs[] = precompiled.sceneData.map(
    (scene, sceneIndex) => {
      const compileScript = (
        script: ScriptEvent[],
        entityType: ScriptBuilderEntityType,
        entity: ScriptBuilderEntity & { symbol: string },
        entityIndex: number,
        loop: boolean,
        lock: boolean,
        scriptKey: string
      ) => {
        let scriptTypeCode = "interact";
        let scriptName = "script";

        if (entityType === "actor") {
          const scriptLookup = {
            script: "interact",
            updateScript: "update",
            hit1Script: "hit1",
            hit2Script: "hit2",
            hit3Script: "hit3",
          };
          scriptTypeCode =
            scriptLookup[scriptKey as keyof typeof scriptLookup] ||
            scriptTypeCode;
        } else if (entityType === "trigger") {
          scriptTypeCode = "interact";
        } else if (entityType === "scene") {
          const scriptLookup = {
            script: "init",
            playerHit1Script: "p_hit1",
            playerHit2Script: "p_hit2",
            playerHit3Script: "p_hit3",
          };
          scriptTypeCode =
            scriptLookup[scriptKey as keyof typeof scriptLookup] ||
            scriptTypeCode;
        }
        scriptName = `${entity.symbol}_${scriptTypeCode}`;

        if (
          isEmptyScript(script) &&
          // Generate scene init for empty script if autoFade is not disabled
          (scriptTypeCode !== "init" || scene.autoFadeSpeed === null)
        ) {
          return null;
        }

        const compiledScript = compileEntityEvents(scriptName, script, {
          scriptEventHandlers,
          scene,
          sceneIndex,
          scenes: precompiled.sceneData,
          music: precompiled.usedMusic,
          sounds: precompiled.usedSounds,
          fonts: precompiled.usedFonts,
          defaultFontId: projectData.settings.defaultFontId,
          sprites: precompiled.usedSprites,
          statesOrder: precompiled.statesOrder,
          stateReferences: precompiled.stateReferences,
          avatars: precompiled.usedAvatars,
          emotes: precompiled.usedEmotes,
          tilesets: precompiled.usedTilesets,
          backgrounds: precompiled.usedBackgrounds,
          customEvents: projectData.customEvents,
          palettes: projectData.palettes,
          settings: projectData.settings,
          variablesLookup,
          variableAliasLookup,
          entityType,
          entityIndex,
          entityScriptKey: scriptKey,
          entity,
          warnings,
          loop,
          lock,
          engineFields: precompiledEngineFields,
          output: [],
          additionalScripts,
          additionalOutput,
          symbols,
          compiledCustomEventScriptCache,
          compiledAssetsCache,
          branch: false,
          isFunction: false,
          debugEnabled,
        });

        output[`${scriptName}.s`] = compiledScript;
        output[`${scriptName}.h`] = compileScriptHeader(scriptName);

        return scriptName;
      };

      const bankSceneEvents = (scene: PrecompiledScene, sceneIndex: number) => {
        // Merge start scripts for actors with scene start script
        const initScript = ([] as ScriptEvent[]).concat(
          scene.actors
            .map((actor) => {
              const actorStartScript = actor.startScript || [];
              if (actorStartScript.length === 0) {
                return [];
              }
              return ([] as ScriptEvent[]).concat(
                {
                  id: "",
                  command: "INTERNAL_SET_CONTEXT",
                  args: {
                    entity: actor,
                    entityType: "actor",
                    entityId: actor.id,
                    scriptKey: "startScript",
                  },
                } as ScriptEvent,
                actorStartScript.filter((event) => event.command !== EVENT_END)
              );
            })
            .flat(),
          scene.script.length > 0
            ? {
                id: "",
                command: "INTERNAL_SET_CONTEXT",
                args: {
                  entity: scene,
                  entityType: "scene",
                  entityId: scene.id,
                  scriptKey: "script",
                },
              }
            : [],
          scene.script || []
        );

        // Inject automatic Scene Fade In if required
        if (scene.autoFadeSpeed !== null) {
          const autoFadeId = calculateAutoFadeEventId(
            initScript,
            customEventsLookup,
            scriptEventHandlers
          );
          const autoFadeIndex = autoFadeId
            ? initScript.findIndex((item) => item.id === autoFadeId)
            : -1;
          const fadeEvent = {
            id: "autofade",
            command: "EVENT_FADE_IN",
            args: {
              speed: scene.autoFadeSpeed,
            },
          };
          if (autoFadeIndex > -1) {
            initScript.splice(autoFadeIndex, 0, fadeEvent);
          } else if (autoFadeId !== "MANUAL") {
            initScript.push(fadeEvent);
          }
        }

        // Compile scene start script
        return compileScript(
          initScript,
          "scene",
          scene,
          sceneIndex,
          false,
          true,
          "script"
        );
      };

      const combineScripts = (
        scripts: {
          parameter: number;
          value: number;
          script: ScriptEvent[];
        }[],
        canCollapse: boolean
      ): ScriptEvent[] => {
        const filteredScripts = scripts.filter(
          (s) => s.script && s.script.length > 0
        );
        if (!canCollapse || filteredScripts.length > 1) {
          return filteredScripts.map((s) => {
            return {
              id: "",
              command: "INTERNAL_IF_PARAM",
              args: {
                parameter: s.parameter,
                value: s.value,
              },
              children: {
                true: s.script,
              },
            };
          });
        } else if (filteredScripts[0]) {
          return filteredScripts[0].script;
        }
        return [];
      };

      const combinedPlayerHitScript = combineScripts(
        [
          { parameter: 0, value: 2, script: scene.playerHit1Script },
          { parameter: 0, value: 4, script: scene.playerHit2Script },
          { parameter: 0, value: 8, script: scene.playerHit3Script },
        ],
        false
      );

      return {
        start: bankSceneEvents(scene, sceneIndex),
        playerHit1: compileScript(
          combinedPlayerHitScript,
          "scene",
          scene,
          sceneIndex,
          false,
          false,
          "playerHit1Script"
        ),
        actorsMovement: scene.actors.map((entity, entityIndex) => {
          if (!entity["updateScript"] || entity["updateScript"].length === 0) {
            return null;
          }
          return compileScript(
            entity["updateScript"],
            "actor",
            entity,
            entityIndex,
            true,
            false,
            "updateScript"
          );
        }),
        actors: scene.actors.map((entity, entityIndex) => {
          if (!entity.collisionGroup) {
            return compileScript(
              entity.script,
              "actor",
              entity,
              entityIndex,
              false,
              true,
              "script"
            );
          }
          const combinedActorScript = combineScripts(
            [
              { parameter: 0, value: 0, script: entity.script },
              { parameter: 0, value: 2, script: entity.hit1Script },
              { parameter: 0, value: 4, script: entity.hit2Script },
              { parameter: 0, value: 8, script: entity.hit3Script },
            ],
            false
          );
          return compileScript(
            combinedActorScript,
            "actor",
            entity,
            entityIndex,
            false,
            false,
            "script"
          );
        }),
        triggers: scene.triggers.map((entity, entityIndex) => {
          const combinedTriggerScript = combineScripts(
            [
              { parameter: 0, value: 1, script: entity.script },
              { parameter: 0, value: 2, script: entity.leaveScript },
            ],
            true
          );

          return compileScript(
            combinedTriggerScript,
            "trigger",
            entity,
            entityIndex,
            false,
            true,
            "script"
          );
        }),
      };
    }
  );

  Object.values(additionalScripts).forEach((additional) => {
    if (!additional) {
      return;
    }

    output[`${additional.symbol}.s`] = additional.compiledScript;
    output[`${additional.symbol}.h`] = compileScriptHeader(additional.symbol);
  });

  (
    Object.values(additionalOutput) as {
      filename: string;
      data: string;
    }[]
  ).forEach((additional) => {
    output[additional.filename] = additional.data;
  });

  precompiled.usedTilesets.forEach((tileset) => {
    output[`${tileset.symbol}.c`] = compileTileset(tileset);
    output[`${tileset.symbol}.h`] = compileTilesetHeader(tileset);
  });

  precompiled.usedBackgroundTilesets.forEach((tileset) => {
    output[`${tileset.symbol}.c`] = compileTileset(tileset);
    output[`${tileset.symbol}.h`] = compileTilesetHeader(tileset);
  });

  precompiled.usedSpriteTilesets.forEach((tileset) => {
    output[`${tileset.symbol}.c`] = compileTileset(tileset);
    output[`${tileset.symbol}.h`] = compileTilesetHeader(tileset);
  });

  // Add palette data
  precompiled.usedPalettes.forEach((palette, paletteIndex) => {
    output[`${paletteSymbol(paletteIndex)}.c`] = compilePalette(
      palette,
      paletteIndex
    );
    output[`${paletteSymbol(paletteIndex)}.h`] = compilePaletteHeader(
      palette,
      paletteIndex
    );
  });

  // Add background map data
  precompiled.usedBackgrounds.forEach((background) => {
    output[`${background.symbol}.c`] = compileBackground(background, {
      color: colorEnabled,
    });
    output[`${background.symbol}.h`] = compileBackgroundHeader(background);
  });

  precompiled.usedTilemaps.forEach((tilemap) => {
    output[`${tilemap.symbol}.c`] = compileTilemap(tilemap);
    output[`${tilemap.symbol}.h`] = compileTilemapHeader(tilemap);
  });

  if (colorEnabled) {
    precompiled.usedTilemapAttrs.forEach((tilemapAttr) => {
      output[`${tilemapAttr.symbol}.c`] = compileTilemapAttrDummy(tilemapAttr);
      output[`${tilemapAttr.symbol}.h`] = compileTilemapAttrHeader(tilemapAttr);
    });
    precompiled.usedTilemapAttrsNES.forEach((tilemapAttrNES: CompiledTilemapData) => {
      output[`${tilemapAttrNES.symbol}.c`] = compileTilemapAttr(tilemapAttrNES);
      output[`${tilemapAttrNES.symbol}.h`] = compileTilemapAttrHeader(tilemapAttrNES);
    });
  }

  // Add sprite data
  precompiled.usedSprites.forEach((sprite, spriteIndex) => {
    output[`${sprite.symbol}.c`] = compileSpriteSheet(sprite, spriteIndex, {
      statesOrder: precompiled.statesOrder,
      stateReferences: precompiled.stateReferences,
    });
    output[`${sprite.symbol}.h`] = compileSpriteSheetHeader(sprite);
  });

  // Add font data
  precompiled.usedFonts.forEach((font) => {
    output[`${font.symbol}.c`] = compileFont(font);
    output[`${font.symbol}.h`] = compileFontHeader(font);
  });

  // Add avatar data
  const avatarFontSize = 16;
  const avatarFonts = chunk(precompiled.usedAvatars, avatarFontSize);
  avatarFonts.forEach((avatarFont, avatarFontIndex) => {
    output[`avatar_font_${avatarFontIndex}.c`] = compileAvatarFont(
      avatarFont,
      avatarFontIndex
    );
    output[`avatar_font_${avatarFontIndex}.h`] =
      compileAvatarFontHeader(avatarFontIndex);
  });

  // Add emote data
  precompiled.usedEmotes.forEach((emote) => {
    output[`${emote.symbol}.c`] = compileEmote(emote);
    output[`${emote.symbol}.h`] = compileEmoteHeader(emote);
  });

  // Add scene data
  precompiled.sceneData.forEach((scene, sceneIndex) => {
    const sceneImage = scene.background;
    const collisionsLength = Math.ceil(sceneImage.width * sceneImage.height);
    const collisions = Array(collisionsLength)
      .fill(0)
      .map((_, index) => {
        return (scene.collisions && scene.collisions[index]) || 0;
      });
    const bgPalette = precompiled.scenePaletteIndexes[scene.id] || 0;
    const actorsPalette = precompiled.sceneActorPaletteIndexes[scene.id] || 0;

    sceneMap[scene.symbol] = {
      id: scene.id,
      name: sceneName(scene, sceneIndex),
      symbol: scene.symbol,
    };

    output[`${scene.symbol}.c`] = compileScene(scene, sceneIndex, {
      bgPalette,
      actorsPalette,
      eventPtrs,
    });
    output[`${scene.symbol}.h`] = compileSceneHeader(scene, sceneIndex);
    output[`${scene.symbol}_collisions.c`] = compileSceneCollisions(
      scene,
      sceneIndex,
      collisions
    );
    output[`${scene.symbol}_collisions.h`] = compileSceneCollisionsHeader(
      scene,
      sceneIndex
    );

    if (scene.actors.length > 0) {
      output[`${scene.symbol}_actors.h`] = compileSceneActorsHeader(
        scene,
        sceneIndex
      );
      output[`${scene.symbol}_actors.c`] = compileSceneActors(
        scene,
        sceneIndex,
        precompiled.usedSprites,
        { eventPtrs }
      );
    }
    if (scene.triggers.length > 0) {
      output[`${scene.symbol}_triggers.h`] = compileSceneTriggersHeader(
        scene,
        sceneIndex
      );
      output[`${scene.symbol}_triggers.c`] = compileSceneTriggers(
        scene,
        sceneIndex,
        { eventPtrs }
      );
    }
    if (scene.sprites.length > 0) {
      output[`${scene.symbol}_sprites.h`] = compileSceneSpritesHeader(
        scene,
        sceneIndex
      );
      output[`${scene.symbol}_sprites.c`] = compileSceneSprites(
        scene,
        sceneIndex
      );
    }
    if (scene.projectiles.length > 0) {
      output[`${scene.symbol}_projectiles.h`] = compileSceneProjectilesHeader(
        scene,
        sceneIndex
      );
      output[`${scene.symbol}_projectiles.c`] = compileSceneProjectiles(
        scene,
        sceneIndex,
        precompiled.usedSprites
      );
    }
  });

  const startScene =
    precompiled.sceneData.find(
      (m) => m.id === projectData.settings.startSceneId
    ) || precompiled.sceneData[0];

  const {
    startX,
    startY,
    startDirection,
    startMoveSpeed = 1,
    startAnimSpeed = 15,
    musicDriver,
    musicTempo,
  } = projectData.settings;

  // Add music data
  output["music_data.h"] = compileMusicHeader(precompiled.usedMusic);
  output["famistudio_settings.i"] = compileFamiStudioSettings(musicTempo);
  await compileMusicTracks(precompiled.usedMusic, {
    engine: musicDriver,
    output,
    tmpPath,
    projectRoot,
    progress,
    warnings,
  });

  // Add sound data
  for (const sound of precompiled.usedSounds) {
    const { src: compiledSoundSrc, header: compiledSoundHeader } =
      await compileSound(sound, {
        projectRoot,
      });
    output[`sounds/${sound.symbol}.c`] = compiledSoundSrc;
    output[`${sound.symbol}.h`] = compiledSoundHeader;
  }

  output["game_globals.i"] = compileGameGlobalsInclude(
    variableAliasLookup,
    precompiled.stateReferences
  );

  output["game_globals.h"] =
    `#ifndef GAME_GLOBALS_H\n#define GAME_GLOBALS_H\n\n` +
    Object.values(variableAliasLookup)
      .map((v) => v?.symbol)
      .map((string, stringIndex) => {
        return `#define ${string} ${stringIndex}\n`;
      })
      .join("") +
    `#define MAX_GLOBAL_VARS ${Object.values(variableAliasLookup).length}\n` +
    `\n` +
    `#endif\n`;

  const variableMap = keyBy(Object.values(variableAliasLookup), "symbol");

  output[`script_engine_init.s`] = compileScriptEngineInit({
    startX,
    startY,
    startDirection,
    startScene,
    startMoveSpeed,
    startAnimSpeed: ensureNumber(startAnimSpeed, 15),
    fonts: precompiled.usedFonts,
    avatarFonts,
    engineFields,
    engineFieldValues: projectData.engineFieldValues,
  });
  output[`data_bootstrap.h`] =
    `#ifndef DATA_PTRS_H\n#define DATA_PTRS_H\n\n` +
    `#include "bankdata.h"\n` +
    `#include "gbs_types.h"\n\n` +
    `extern const INT16 start_scene_x;\n` +
    `extern const INT16 start_scene_y;\n` +
    `extern const direction_e start_scene_dir;\n` +
    `extern const far_ptr_t start_scene;\n` +
    `extern const UBYTE start_player_move_speed;\n` +
    `extern const UBYTE start_player_anim_tick;\n\n` +
    `extern const far_ptr_t ui_fonts[];\n\n` +
    `void bootstrap_init(void) BANKED;\n\n` +
    `#endif\n`;

  output[`scene_types.h`] =
    `#ifndef SCENE_TYPES_H\n#define SCENE_TYPES_H\n\n` +
    `typedef enum {\n` +
    sceneTypes
      .map((t, i) => {
        return `    SCENE_TYPE_${t.key.toUpperCase()}${i === 0 ? " = 0" : ""}`;
      })
      .join(",\n") +
    `\n} scene_type_e;\n` +
    `#endif\n`;

  output[`../states/states_ptrs.s`] =
    `.include "macro.i"\n\n` +
    `.area _CODE\n\n` +
    `_state_start_fns::\n` +
    sceneTypes
      .map((t) => {
        return `    IMPORT_FAR_PTR _${t.key.toLowerCase()}_init`;
      })
      .join("\n") +
    `\n\n` +
    `_state_update_fns::\n` +
    sceneTypes
      .map((t) => {
        return `    IMPORT_FAR_PTR _${t.key.toLowerCase()}_update`;
      })
      .join("\n");

  output[`states_defines.h`] =
    `#ifndef STATES_DEFINES_H\n#define STATES_DEFINES_H\n\n` +
    // Add define fields from engineFields
    engineFields
      .filter(
        // Add define types without explict file set to data/data_bootstrap.h
        (engineField) => engineField.cType === "define" && !engineField.file
      )
      .map((engineField, defineIndex, defineFields) => {
        const engineValue = projectData.engineFieldValues.find(
          (v) => v.id === engineField.key
        );
        const value =
          engineValue && engineValue.value !== undefined
            ? engineValue.value
            : engineField.defaultValue;
        return `#define ${String(engineField.key).padEnd(32, " ")} ${value}${
          defineIndex === defineFields.length - 1 ? "\n\n" : "\n"
        }`;
      })
      .join("") +
    `#endif\n`;

  output[`game_signature.c`] = compileSaveSignature(
    JSON.stringify(projectData)
  );

  return {
    files: output,
    sceneMap,
    variableMap,
  };
};

export default compile;
