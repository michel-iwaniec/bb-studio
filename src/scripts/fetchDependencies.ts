import { remove, writeFile } from "fs-extra";
import Path from "path";
import AdmZip from "adm-zip";
import spawn from "../../src/lib/helpers/cli/spawn";

const buildToolsRoot = Path.join(
  Path.normalize(`${__dirname}/../../`),
  "buildTools"
);

const dependencies = {
  "darwin-arm64": {
    gbdk: {
      url: "https://github.com/gbdk-2020/gbdk-2020/releases/download/4.3.0/gbdk-macos-arm64.tar.gz",
      type: "targz",
    },
  },
  "darwin-x64": {
    gbdk: {
      url: "https://github.com/gbdk-2020/gbdk-2020/releases/download/4.3.0/gbdk-macos.tar.gz",
      type: "targz",
    },
  },
  "linux-x64": {
    gbdk: {
      url: "https://github.com/gbdk-2020/gbdk-2020/releases/download/4.3.0/gbdk-linux64.tar.gz",
      type: "targz",
    },
  },
  "win32-ia32": {
    gbdk: {
      url: "https://github.com/gbdk-2020/gbdk-2020/releases/download/4.3.0/gbdk-win32.zip",
      type: "zip",
    },
  },
  "win32-x64": {
    gbdk: {
      url: "https://github.com/gbdk-2020/gbdk-2020/releases/download/4.3.0/gbdk-win64.zip",
      type: "zip",
    },
  },
} as const;

type Arch = keyof typeof dependencies;

const fetchAll = process.argv.includes("--all");

const archs = Object.keys(dependencies) as Array<Arch>;
const localArch = `${process.platform}-${process.arch}`;

const extractTarGz = async (
  archivePath: string,
  outputDir: string
): Promise<void> => {
  const res = spawn("tar", ["-zxf", archivePath, "-C", outputDir], {}, {});
  await res.completed;
};

const extractZip = async (
  archivePath: string,
  outputDir: string
): Promise<void> => {
  const zip = new AdmZip(archivePath);
  await zip.extractAllTo(outputDir, true);
};

export const fetchGBDKDependency = async (arch: Arch) => {
  const { url, type } = dependencies[arch].gbdk;
  const response = await fetch(url);
  const buffer = await response.arrayBuffer(); // Get a Buffer from the response
  const data = Buffer.from(buffer);
  const tmpPath = Path.join(buildToolsRoot, "tmp.data");
  await writeFile(tmpPath, data);

  const gbdkArchPath = Path.join(buildToolsRoot, arch);

  if (type === "targz") {
    await extractTarGz(tmpPath, gbdkArchPath);
  } else {
    await extractZip(tmpPath, gbdkArchPath);
  }

  await remove(tmpPath);
};

const main = async () => {
  for (const arch of archs) {
    if (fetchAll || arch === localArch) {
      await fetchGBDKDependency(arch);
    }
  }
};

main();
