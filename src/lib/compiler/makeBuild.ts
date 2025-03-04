import fs from "fs-extra";
import os from "os";
import {
  buildLinkFile,
  buildLinkFlags,
  getBuildCommands,
  getPackFiles,
} from "./buildMakeScript";
import { cacheObjData, fetchCachedObjData } from "./objCache";
import ensureBuildTools from "./ensureBuildTools";
import spawn, { ChildProcess } from "lib/helpers/cli/spawn";
import { gbspack } from "./gbspack";
import l10n from "shared/lib/lang/l10n";
import type { ProjectData } from "store/features/project/projectActions";

type MakeOptions = {
  buildRoot: string;
  tmpPath: string;
  data: ProjectData;
  buildType: "rom" | "web" | "pocket";
  debug: boolean;
  progress: (msg: string) => void;
  warnings: (msg: string) => void;
};

const cpuCount = os.cpus().length;

const makeBuild = async ({
  buildRoot = "/tmp",
  tmpPath = "/tmp",
  data,
  debug = false,
  buildType = "rom",
  progress = (_msg) => {},
  warnings = (_msg) => {},
}: MakeOptions) => {
  const env = Object.create(process.env);
  const { settings } = data;
  const colorEnabled = settings.colorMode !== "mono";
  const sgbEnabled = settings.sgbEnabled && settings.colorMode !== "color";
  const colorOnly = settings.colorMode === "color";
  const targetPlatform = "nes"; //buildType === "pocket" ? "pocket" : "gb";
  const batterylessEnabled = settings.batterylessEnabled && buildType !== "web";
  
  console.log(`targetPlatform = ${targetPlatform}`); // DEBUGHACK

  const buildToolsPath = await ensureBuildTools(tmpPath);
  const buildToolsVersion = await fs.readFile(
    `${buildToolsPath}/tools_version`,
    "utf8"
  );

  env.PATH = [`${buildToolsPath}/gbdk/bin`, env.PATH].join(":");
  env.GBDKDIR = `${buildToolsPath}/gbdk/`;
  env.GBS_TOOLS_VERSION = buildToolsVersion;
  env.TARGET_PLATFORM = targetPlatform;

  env.CART_TYPE = settings.cartType || "mbc5";
  env.TMP = tmpPath;
  env.TEMP = tmpPath;
  if (colorEnabled) {
    env.COLOR = true;
  }
  if (sgbEnabled) {
    env.SGB = true;
  }
  if (batterylessEnabled) {
    env.BATTERYLESS = true;
  }
  env.COLOR_MODE = settings.colorMode;
  env.MUSIC_DRIVER = settings.musicDriver;
  if (debug) {
    env.DEBUG = true;
  }
  //if (settings.musicDriver === "huge") {
  //  env.MUSIC_DRIVER = "HUGE_TRACKER";
  //} else {
  //  env.MUSIC_DRIVER = "GBT_PLAYER";
  //}
  env.MUSIC_DRIVER = "FAMISTUDIO";
  if (settings.cartType === "mbc3") {
    env.RUMBLE_ENABLE = 0x20;
  } else {
    env.RUMBLE_ENABLE = 0x08;
  }

  // Populate /obj with cached data
  await fetchCachedObjData(buildRoot, tmpPath, env);

  // Compile Source Files
  console.log(`Calling getBuildCommands...`); // DEBUGHACK
  const makeCommands = await getBuildCommands(buildRoot, {
    colorEnabled,
    sgb: sgbEnabled,
    musicDriver: settings.musicDriver,
    batteryless: batterylessEnabled,
    debug,
    platform: process.platform,
    targetPlatform,
    cartType: settings.cartType,
  });
  console.log(`Done calling getBuildCommands!`); // DEBUGHACK
  const options = {
    cwd: buildRoot,
    env,
    shell: true,
  };

  // Build source files in parallel
  const childSet = new Set<ChildProcess>();
  const concurrency = cpuCount;
  await Promise.all(
    Array(concurrency)
      .fill(makeCommands.entries())
      .map(async (cursor) => {
        for (const [_, makeCommand] of cursor) {
          try {
            progress(makeCommand.label);
          } catch (e) {
            for (const child of childSet) {
              child.kill();
            }
            throw e;
          }
          console.log(`Spawn: makeCommand.command = ${makeCommand.command}, makeCommand.args = ${makeCommand.args}`); // DEBUGHACK
          const { child, completed } = spawn(
            makeCommand.command,
            makeCommand.args,
            options,
            {
              onLog: (msg) => warnings(msg), // LCC writes errors to stdout
              onError: (msg) => warnings(msg),
            }
          );
          childSet.add(child);
          await completed;
          console.log(`-- Completed: makeCommand.command = ${makeCommand.command}, makeCommand.args = ${makeCommand.args}`);
          childSet.delete(child);
        }
      })
  );

  // GBSPack ---

  progress(`${l10n("COMPILER_PACKING")}...`);
  const { cartSize } = await gbspack(await getPackFiles(buildRoot), {
    bankOffset: 3, //1, // gbdk-nes: Avoid packing in VM bank (bank 2) for alignment reasons
    filter: 255,
    extension: "rel",
    additional: batterylessEnabled ? 4 : 0,
    reserve:
      settings.musicDriver !== "huge"
        ? {
            // Reserve space in bank1 for gbt_player.lib
            1: 0x800,
          }
        : {},
  });

  // Link ROM ---

  progress(`${l10n("COMPILER_LINKING")}...`);
  const linkFile = await buildLinkFile(buildRoot, cartSize);
  const linkFilePath = `${buildRoot}/obj/linkfile.lk`;
  await fs.writeFile(linkFilePath, linkFile);

  const linkCommand =
    process.platform === "win32"
      ? `..\\_gbstools\\gbdk\\bin\\lcc.exe`
      : `../_gbstools/gbdk/bin/lcc`;
  const linkArgs = buildLinkFlags(
    linkFilePath,
    data.name || "GBStudio",
    settings.cartType,
    colorEnabled,
    sgbEnabled,
    colorOnly,
    settings.musicDriver,
    debug,
    targetPlatform
  );

  console.log(`Spawn: linkCommand = ${linkCommand}, linkArgs = ${linkArgs}`); // DEBUGHACK
  const { completed: linkCompleted } = spawn(linkCommand, linkArgs, options, {
    onLog: (msg) => progress(msg),
    onError: (msg) => {
      if (msg.indexOf("Converted build") > -1) {
        return;
      }
      warnings(msg);
    },
  });
  await linkCompleted;
  console.log(`Link completed!`); // DEBUGHACK

  // Store /obj in cache
  await cacheObjData(buildRoot, tmpPath, env);
};

export default makeBuild;
