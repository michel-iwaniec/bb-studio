import childProcess from "child_process";
import Path from "path";
import { checksumFile, checksumString } from "lib/helpers/checksum";
import { readFile, ensureDir, pathExists, writeFile, unlink } from "fs-extra";
import ensureBuildTools from "./ensureBuildTools";
import { exportToC, loadUGESong } from "shared/lib/uge/ugeHelper";
import { assetFilename } from "shared/lib/helpers/assets";
import spawn, { ChildProcess } from "lib/helpers/cli/spawn";

export interface PrecompiledMusicTrack {
  id: string;
  name: string;
  dataName: string;
  filename: string;
  settings: {
    disableSpeedConversion?: boolean;
  };
}

interface CompileMusicOptions {
  tmpPath: string;
  projectRoot: string;
  engine: "gbt" | "huge";
  output: Record<string, string>;
  progress: (msg: string) => void;
  warnings: (msg: string) => void;
}

interface CompileModTrackOptions {
  projectRoot: string;
  buildToolsPath: string;
  buildToolsVersion: string;
  cacheRoot: string;
  progress: (msg: string) => void;
  warnings: (msg: string) => void;
}

interface CompileHugeTrackOptions {
  projectRoot: string;
  buildToolsPath: string;
  buildToolsVersion: string;
  cacheRoot: string;
  progress: (msg: string) => void;
  warnings: (msg: string) => void;
}

const compileModTrack = async (
  track: PrecompiledMusicTrack,
  {
    projectRoot,
    buildToolsPath,
    buildToolsVersion,
    cacheRoot,
    progress = (_msg) => {},
    warnings = (_msg) => {},
  }: CompileModTrackOptions
): Promise<string> => {
  const env = Object.create(process.env);
  env.PATH = [`${buildToolsPath}/mod2gbt`, env.PATH].join(":");

  const command =
    process.platform === "win32"
      ? `"${buildToolsPath}\\mod2gbt\\mod2gbt.exe"`
      : "mod2gbt";

  const modPath = assetFilename(projectRoot, "music", track);

  const checksum = await checksumFile(modPath);
  const buildChecksum = checksumString(
    checksum + buildToolsVersion + JSON.stringify(track.settings)
  );
  const cachedFilePath = `${cacheRoot}/${buildChecksum}`;

  if (await pathExists(cachedFilePath)) {
    return readFile(cachedFilePath, "utf8");
  }

  const outputFile = "output.c";
  const tmpFilePath = `${cacheRoot}/${outputFile}`;

  const args = [`"${modPath}"`, "MUSICTRACKNAME", "255"];

  if (track.settings.disableSpeedConversion) {
    args.push("-speed");
  }

  progress(`${command} ${args.join(" ")}`);

  const options = {
    cwd: cacheRoot,
    env,
    shell: true,
  };

  await new Promise<void>((resolve, reject) => {
    const child = childProcess.spawn(command, args, options);

    child.on("error", (err) => {
      warnings(err.toString());
    });

    child.stdout.on("data", (data) => {
      const lines = data.toString().split("\n");
      lines.forEach((line: string) => {
        progress(line);
      });
    });

    child.stderr.on("data", (data) => {
      const lines = data.toString().split("\n");
      lines.forEach((line: string) => {
        warnings(line);
      });
    });

    child.on("close", (code) => {
      if (code === 0) resolve();
      else reject(code);
    });
  });

  const output = (await readFile(tmpFilePath, "utf8")).replace(
    /#pragma bank=255/,
    `#pragma bank 255\n\n#include "gbs_types.h"\n\nBANKREF(MUSICTRACKNAME_Data)\n`
  );

  await writeFile(cachedFilePath, output);
  await unlink(tmpFilePath);

  return output;
};

const compileModTracks = async (
  tracks: PrecompiledMusicTrack[],
  {
    tmpPath,
    projectRoot,
    output,
    progress = (_msg) => {},
    warnings = (_msg) => {},
  }: CompileMusicOptions
): Promise<void> => {
  const buildToolsPath = await ensureBuildTools(tmpPath);
  const cacheRoot = Path.normalize(`${tmpPath}/_gbscache/music`);
  ensureDir(cacheRoot);
  const buildToolsVersion = await readFile(
    `${buildToolsPath}/tools_version`,
    "utf8"
  );

  for (const track of tracks) {
    const fileData = await compileModTrack(track, {
      projectRoot,
      buildToolsPath,
      buildToolsVersion,
      cacheRoot,
      progress,
      warnings,
    });
    output[`music/${track.dataName}_Data.c`] = fileData.replace(
      /MUSICTRACKNAME/g,
      track.dataName
    );
  }
};

const compileUgeTrack = async (
  track: PrecompiledMusicTrack,
  { projectRoot, buildToolsPath }: CompileHugeTrackOptions
): Promise<string> => {
  const ugePath = assetFilename(projectRoot, "music", track);
  const data = await readFile(ugePath);
  const song = loadUGESong(new Uint8Array(data).buffer);
  if (song) {   
    // Convert FamiStudio file named the same as .uge file
    var fmsFile = Path.basename(ugePath, ".uge") + ".fms";
    var asmFile = Path.basename(ugePath, ".uge") + ".s";
    var ugeDir = Path.dirname(ugePath);
    
    var fmsPath = Path.join(ugeDir, fmsFile);
    var asmPath = Path.join(ugeDir, asmFile);

    console.log(`compileUgeTrack: track.dataName = ${track.dataName}, ugePath = ${ugePath}, asmPath = ${asmPath}`); 


  if(!(await pathExists(fmsPath))) {
    //let arrayName = `${track.dataName}_Data`
    //return `#pragma bank 255\nconst void __at(5) __bank_${arrayName};\nextern const unsigned char ${arrayName}; // DUMMY DATA!!!\n`;
    fmsPath = Path.join(ugeDir, "dummy.fms");
  }

  const env = Object.create(process.env);
  const options = {
    cwd: "", //"/tmp",
    env,
    shell: true,
  };

  //const buildToolsPath = await ensureBuildTools(tmpPath);
  const FamiStudioDll =
    process.platform === "win32"
      ? `"${buildToolsPath}\\famistudio\\FamiStudio.dll"`
      : `"${buildToolsPath}/famistudio/FamiStudio.dll"`;

  const args = [`"${fmsPath}"`, "famistudio-asm-export", `"${asmPath}"`, "famistudio-asm-seperate-song-pattern", `${Path.basename(fmsPath) }_42`, "-famistudio-asm-format:sdas"];
  console.log(`Executing "dotnet ${FamiStudioDll} ${args}""`);

  await new Promise<void>((resolve, reject) => {
    const child = childProcess.spawn(`dotnet ${FamiStudioDll}`,
                                     args,
                                     options);

    child.on("error", (err) => {
      //warnings(err.toString());
    });

    child.stdout.on("data", (data) => {
      //const lines = data.toString().split("\n");
      //lines.forEach((line: string) => {
      //  progress(line);
      //});
    });

    child.stderr.on("data", (data) => {
      //const lines = data.toString().split("\n");
      //lines.forEach((line: string) => {
      //  warnings(line);
      //});
    });

    child.on("close", (code) => {
      if (code === 0) resolve();
      else reject(code);
    });
  });

    var arrayName = `${track.dataName}_Data`
    //return `#pragma bank 255\nconst void __at(5) __bank_${arrayName};\nextern const unsigned char ${arrayName}; // DUMMY DATA!!!\n`; // gbdk-nes: Bypass for FamiStudio!
    
    
    const asmBuffer = await readFile(asmPath);
    var asmText: string = asmBuffer.toString();
    asmText = asmText.replace(".globl _music_data_\r\n", ""); // Remove .globl for unnamed songs to avoid link errors (win)
    asmText = asmText.replace(".globl music_data_\r\n", ""); // Remove .globl for unnamed songs to avoid link errors (win)
    asmText = asmText.replace(".globl _music_data_\n", ""); // Remove .globl for unnamed songs to avoid link errors (linux)
    asmText = asmText.replace(".globl music_data_\n", ""); // Remove .globl for unnamed songs to avoid link errors (linux)
    return `#pragma bank 255\nconst void __at(5) __bank_${arrayName};\nvoid dummyFunc${arrayName}() {\n__asm\nFAMISTUDIO_CFG_C_BINDINGS = 1\n.area _CODE_5\n_${arrayName}::\nFAMISTUDIO_DPCM_PTR=0xFF\n${asmText}\n__endasm;\n}\n`; // gbdk-nes: Bypass for FamiStudio!
    
    //return exportToC(song, track.dataName);
  } else {
    return "// No song found";
  }
};

const compileUgeTracks = async (
  tracks: PrecompiledMusicTrack[],
  {
    tmpPath,
    projectRoot,
    output,
    progress = (_msg) => {},
    warnings = (_msg) => {},
  }: CompileMusicOptions
): Promise<void> => {
  console.log(`compileUgeTracks: ${tmpPath}`);
  const buildToolsPath = await ensureBuildTools(tmpPath);
  const cacheRoot = Path.normalize(`${tmpPath}/_gbscache/music`);
  ensureDir(cacheRoot);
  const buildToolsVersion = await readFile(
    `${buildToolsPath}/tools_version`,
    "utf8"
  );

  console.log(`Processing tracks: ${tracks}`);
  for (const track of tracks) {
    console.log(`Processing music/${track.dataName}_Data.c`);
    output[`music/${track.dataName}_Data.c`] = await compileUgeTrack(track, {
      projectRoot,
      buildToolsPath,
      buildToolsVersion,
      cacheRoot,
      progress,
      warnings,
    });
  }
};

export const compileMusicHeader = (tracks: PrecompiledMusicTrack[]) => {
  return `#ifndef MUSIC_DATA_H
#define MUSIC_DATA_H

${tracks
  .map(
    (track) => `extern const void __bank_${track.dataName}_Data;
extern const void ${track.dataName}_Data;`
  )
  .join("\n")}

#endif
`;
};

export const compileFamiStudioSettings = (tempoMode: string) => {
  if (tempoMode === "famitracker_tempo") {
    return ["FAMISTUDIO_USE_FAMITRACKER_TEMPO = 1",
            "FAMISTUDIO_USE_FAMITRACKER_DELAYED_NOTES_OR_CUTS = 1"].join("\n");
  }
  else {
    return ["FAMISTUDIO_USE_FAMITRACKER_TEMPO = 0",
            "FAMISTUDIO_USE_FAMITRACKER_DELAYED_NOTES_OR_CUTS = 0"].join("\n");
  }
}

export const compileMusicTracks = (
  tracks: PrecompiledMusicTrack[],
  options: CompileMusicOptions
) => {
  if (options.engine === "gbt") {
    console.log("Calling compileModTracks(tracks, options)");
    return compileModTracks(tracks, options);
  } else {
    console.log("Calling compileUgeTracks(tracks, options)");
    return compileUgeTracks(tracks, options);
  }
};
