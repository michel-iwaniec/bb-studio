import fs from "fs-extra";
import path from "path";
import uuid from "uuid/v4";
import loadAllBackgroundData from "./loadBackgroundData";
import loadAllSpriteData from "./loadSpriteData";
import loadAllMusicData from "./loadMusicData";
import loadAllFontData from "./loadFontData";
import loadAllAvatarData from "./loadAvatarData";
import loadAllEmoteData from "./loadEmoteData";
import loadAllSoundData from "./loadSoundData";
import loadAllScriptEventHandlers, {
  ScriptEventDef,
} from "./loadScriptEventHandlers";
import migrateProject from "./migrateProject";
import type { ProjectData } from "store/features/project/projectActions";
import type {
  EngineFieldSchema,
  SceneTypeSchema,
} from "store/features/engine/engineState";
import type { Asset } from "shared/lib/helpers/assets";
import keyBy from "lodash/keyBy";
import { cloneDictionary } from "lib/helpers/clone";
import { Dictionary } from "@reduxjs/toolkit";
import { loadEngineFields } from "lib/project/engineFields";
import { loadSceneTypes } from "lib/project/sceneTypes";
import loadAllTilesetData from "lib/project/loadTilesetData";
import { rgb555_to_rgb222, rgb888Hex_to_nesHex } from "lib/compiler/rgb_to_nes";
import { rgb222_to_nes } from "lib/compiler/rgb_to_nes";
import { rgb555_to_nes } from "lib/compiler/rgb_to_nes";
import { rgb888_to_nes } from "lib/compiler/rgb_to_nes";
import { hexDec } from "shared/lib/helpers/8bit";

const toUnixFilename = (filename: string) => {
  return filename.replace(/\\/g, "/");
};

const toAssetFilename = (elem: Asset) => {
  return (elem.plugin ? `${elem.plugin}/` : "") + toUnixFilename(elem.filename);
};

const indexByFilename = <T extends Asset>(arr: T[]): Record<string, T> =>
  keyBy(arr || [], toAssetFilename);

const sortByName = (a: { name: string }, b: { name: string }) => {
  const aName = a.name.toUpperCase();
  const bName = b.name.toUpperCase();
  if (aName < bName) {
    return -1;
  }
  if (aName > bName) {
    return 1;
  }
  return 0;
};

const loadProject = async (
  projectPath: string
): Promise<{
  data: ProjectData;
  scriptEventDefs: Dictionary<ScriptEventDef>;
  engineFields: EngineFieldSchema[];
  sceneTypes: SceneTypeSchema[];
  modifiedSpriteIds: string[];
  isMigrated: boolean;
}> => {
  const projectRoot = path.dirname(projectPath);

  const scriptEventDefs = await loadAllScriptEventHandlers(projectRoot);
  const engineFields = await loadEngineFields(projectRoot);
  const sceneTypes = await loadSceneTypes(projectRoot);

  const originalJson = await fs.readJson(projectPath);

  const { _version: originalVersion, _release: originalRelease } = originalJson;

  const json = migrateProject(
    originalJson,
    projectRoot,
    scriptEventDefs
  ) as ProjectData;

  const isMigrated =
    json._version !== originalVersion || json._release !== originalRelease;

  const [
    backgrounds,
    sprites,
    music,
    sounds,
    fonts,
    avatars,
    emotes,
    tilesets,
  ] = await Promise.all([
    loadAllBackgroundData(projectRoot),
    loadAllSpriteData(projectRoot),
    loadAllMusicData(projectRoot),
    loadAllSoundData(projectRoot),
    loadAllFontData(projectRoot),
    loadAllAvatarData(projectRoot),
    loadAllEmoteData(projectRoot),
    loadAllTilesetData(projectRoot),
  ]);

  // Merge stored backgrounds data with file system data
  const oldBackgroundByFilename = indexByFilename(json.backgrounds || []);

  const fixedBackgroundIds = backgrounds
    .map((background) => {
      const oldBackground =
        oldBackgroundByFilename[toAssetFilename(background)];
      if (oldBackground) {
        return {
          ...background,
          id: oldBackground.id,
          symbol:
            oldBackground?.symbol !== undefined
              ? oldBackground.symbol
              : background.symbol,
          tileColors:
            oldBackground?.tileColors !== undefined
              ? oldBackground.tileColors
              : [],
          autoColor:
            oldBackground?.autoColor !== undefined
              ? oldBackground.autoColor
              : false,
        };
      }
      return {
        ...background,
        tileColors: [],
      };
    })
    .sort(sortByName);

  // Merge stored sprite data with file system data
  const oldSpriteByFilename = indexByFilename(json.spriteSheets || []);
  const modifiedSpriteIds: string[] = [];

  const fixedSpriteIds = sprites
    .map((sprite) => {
      const oldSprite = oldSpriteByFilename[toAssetFilename(sprite)];
      const oldData = oldSprite || {};
      const id = oldData.id || sprite.id;

      if (!oldSprite || !oldSprite.states || oldSprite.numTiles === undefined) {
        modifiedSpriteIds.push(id);
      }

      return {
        ...sprite,
        ...oldData,
        id,
        symbol: oldData?.symbol !== undefined ? oldData.symbol : sprite.symbol,
        filename: sprite.filename,
        name: oldData.name || sprite.name,
        canvasWidth: oldData.canvasWidth || 32,
        canvasHeight: oldData.canvasHeight || 32,
        states: (
          oldData.states || [
            {
              id: uuid(),
              name: "",
              animationType: "multi_movement",
              flipLeft: true,
            },
          ]
        ).map((oldState) => {
          return {
            ...oldState,
            animations: Array.from(Array(8)).map((_, animationIndex) => ({
              id:
                (oldState.animations &&
                  oldState.animations[animationIndex] &&
                  oldState.animations[animationIndex].id) ||
                uuid(),
              frames: (oldState.animations &&
                oldState.animations[animationIndex] &&
                oldState.animations[animationIndex].frames) || [
                {
                  id: uuid(),
                  tiles: [],
                },
              ],
            })),
          };
        }),
      };
    })
    .sort(sortByName);

  // Merge stored music data with file system data
  const oldMusicByFilename = indexByFilename(json.music || []);

  const fixedMusicIds = music
    .map((track) => {
      const oldTrack = oldMusicByFilename[toAssetFilename(track)];
      if (oldTrack) {
        return {
          ...track,
          id: oldTrack.id,
          symbol:
            oldTrack?.symbol !== undefined ? oldTrack.symbol : track.symbol,
          settings: {
            ...oldTrack.settings,
          },
        };
      }
      return track;
    })
    .sort(sortByName);

  // Merge stored sound effect data with file system data
  const oldSoundByFilename = indexByFilename(json.sounds || []);

  const fixedSoundIds = sounds
    .map((sound) => {
      const oldSound = oldSoundByFilename[toAssetFilename(sound)];
      if (oldSound) {
        return {
          ...sound,
          id: oldSound.id,
          symbol:
            oldSound?.symbol !== undefined ? oldSound.symbol : sound.symbol,
        };
      }
      return sound;
    })
    .sort(sortByName);

  // Merge stored fonts data with file system data
  const oldFontByFilename = indexByFilename(json.fonts || []);

  const fixedFontIds = fonts
    .map((font) => {
      const oldFont = oldFontByFilename[toAssetFilename(font)];
      if (oldFont) {
        return {
          ...font,
          id: oldFont.id,
          symbol: oldFont?.symbol !== undefined ? oldFont.symbol : font.symbol,
        };
      }
      return font;
    })
    .sort(sortByName);

  // Merge stored avatars data with file system data
  const oldAvatarByFilename = indexByFilename(json.avatars || []);

  const fixedAvatarIds = avatars
    .map((avatar) => {
      const oldAvatar = oldAvatarByFilename[toAssetFilename(avatar)];
      if (oldAvatar) {
        return {
          ...avatar,
          id: oldAvatar.id,
        };
      }
      return avatar;
    })
    .sort(sortByName);

  // Merge stored emotes data with file system data
  const oldEmoteByFilename = indexByFilename(json.emotes || []);

  const fixedEmoteIds = emotes
    .map((emote) => {
      const oldEmote = oldEmoteByFilename[toAssetFilename(emote)];
      if (oldEmote) {
        return {
          ...emote,
          id: oldEmote.id,
          symbol:
            oldEmote?.symbol !== undefined ? oldEmote.symbol : emote.symbol,
        };
      }
      return emote;
    })
    .sort(sortByName);

  // Merge stored tilesets data with file system data
  const oldTilesetByFilename = indexByFilename(json.tilesets || []);

  const fixedTilesetIds = tilesets
    .map((tileset) => {
      const oldTileset = oldTilesetByFilename[toAssetFilename(tileset)];
      if (oldTileset) {
        return {
          ...tileset,
          id: oldTileset.id,
          symbol:
            oldTileset?.symbol !== undefined
              ? oldTileset.symbol
              : tileset.symbol,
        };
      }
      return tileset;
    })
    .sort(sortByName);

  const addMissingEntityId = <T extends { id: string }>(entity: T) => {
    if (!entity.id) {
      return {
        ...entity,
        id: uuid(),
      };
    }
    return entity;
  };

  // Fix ids on actors and triggers
  const fixedScenes = (json.scenes || []).map((scene) => {
    return {
      ...scene,
      actors: scene.actors.map(addMissingEntityId),
      triggers: scene.triggers.map(addMissingEntityId),
    };
  });

  const fixedCustomEvents = (json.customEvents || []).map(addMissingEntityId);

  const defaultPalettes = [
    {
      id: "default-bg-1",
      name: "Default BG 1",
      colors: ["F8E8C8", "D89048", "A82820", "301850"],
      nesColors: [rgb888Hex_to_nesHex("F8E8C8"), rgb888Hex_to_nesHex("D89048"), rgb888Hex_to_nesHex("A82820"), rgb888Hex_to_nesHex("301850")],
    },
    {
      id: "default-bg-2",
      name: "Default BG 2",
      colors: ["E0F8A0", "78C838", "488818", "081800"],
      nesColors: [rgb888Hex_to_nesHex("E0F8A0"), rgb888Hex_to_nesHex("78C838"), rgb888Hex_to_nesHex("488818"), rgb888Hex_to_nesHex("081800")],
    },
    {
      id: "default-bg-3",
      name: "Default BG 3",
      colors: ["F8D8A8", "E0A878", "785888", "002030"],
      nesColors: [rgb888Hex_to_nesHex("F8D8A8"), rgb888Hex_to_nesHex("E0A878"), rgb888Hex_to_nesHex("785888"), rgb888Hex_to_nesHex("002030")],
    },
    {
      id: "default-bg-4",
      name: "Default BG 4",
      colors: ["B8D0D0", "D880D8", "8000A0", "380000"],
      nesColors: [rgb888Hex_to_nesHex("B8D0D0"), rgb888Hex_to_nesHex("D880D8"), rgb888Hex_to_nesHex("8000A0"), rgb888Hex_to_nesHex("380000")],
    },
    {
      id: "default-bg-5",
      name: "Default BG 5",
      colors: ["F8F8B8", "90C8C8", "486878", "082048"],
      nesColors: [rgb888Hex_to_nesHex("F8F8B8"), rgb888Hex_to_nesHex("90C8C8"), rgb888Hex_to_nesHex("486878"), rgb888Hex_to_nesHex("082048")],
    },
    {
      id: "default-bg-6",
      name: "Default BG 6",
      colors: ["F8D8B0", "78C078", "688840", "583820"],
      nesColors: [rgb888Hex_to_nesHex("F8D8B0"), rgb888Hex_to_nesHex("78C078"), rgb888Hex_to_nesHex("688840"), rgb888Hex_to_nesHex("583820")],
    },
    {
      id: "default-sprite",
      name: "Default Sprites",
      colors: ["F8F0E0", "D88078", "B05010", "000000"],
      nesColors: ["37", "17", "17", "1D"],
    },
    {
      id: "default-ui",
      name: "Default UI",
      colors: ["F8F8B8", "90C8C8", "486878", "082048"],
      nesColors: [rgb888Hex_to_nesHex("F8F8B8"), rgb888Hex_to_nesHex("90C8C8"), rgb888Hex_to_nesHex("486878"), rgb888Hex_to_nesHex("082048")],
    },
  ] as {
    id: string;
    name: string;
    colors: [string, string, string, string];
    nesColors: [string, string, string, string];
  }[];

  const fixedPalettes = (json.palettes || []).map(addMissingEntityId);

  for (let i = 0; i < defaultPalettes.length; i++) {
    const defaultPalette = defaultPalettes[i];
    const existingPalette = fixedPalettes.find(
      (p) => p.id === defaultPalette.id
    );
    if (existingPalette) {
      existingPalette.defaultName = defaultPalette.name;
      existingPalette.defaultColors = defaultPalette.colors;
    } else {
      fixedPalettes.push({
        ...defaultPalette,
        defaultName: defaultPalette.name,
        defaultColors: defaultPalette.colors,
      });
    }
  }

  const fixedEngineFieldValues = json.engineFieldValues || [];

  // gbdk-nes: Override project "colorOnly" setting to be "mixed" instead, as distinction doesn't make sense for NES
  json.settings.colorMode = "mixed";

  // gbdk-nes: Convert RGB values to NES PPU colors
  for (let i = 0; i < fixedPalettes.length; i++) {
    const c = fixedPalettes[i].colors;
    const d = fixedPalettes[i].defaultColors || ["F8E8C8", "D89048", "A82820", "301850"];
    if (!fixedPalettes[i].nesColors) {
      fixedPalettes[i].nesColors = [rgb888Hex_to_nesHex(c[0]),
                                    rgb888Hex_to_nesHex(c[1]),
                                    rgb888Hex_to_nesHex(c[2]),
                                    rgb888Hex_to_nesHex(c[3])];
      fixedPalettes[i].defaultNesColors = [rgb888Hex_to_nesHex(d[0]),
                                           rgb888Hex_to_nesHex(d[1]),
                                           rgb888Hex_to_nesHex(d[2]),
                                           rgb888Hex_to_nesHex(d[3])];
    }
  }

  return {
    data: {
      ...json,
      backgrounds: fixedBackgroundIds,
      spriteSheets: fixedSpriteIds,
      music: fixedMusicIds,
      sounds: fixedSoundIds,
      fonts: fixedFontIds,
      avatars: fixedAvatarIds,
      emotes: fixedEmoteIds,
      tilesets: fixedTilesetIds,
      scenes: fixedScenes,
      customEvents: fixedCustomEvents,
      palettes: fixedPalettes,
      engineFieldValues: fixedEngineFieldValues,
    },
    modifiedSpriteIds,
    isMigrated,
    scriptEventDefs: cloneDictionary(scriptEventDefs),
    engineFields,
    sceneTypes,
  };
};

export default loadProject;
