import commonjs from "@rollup/plugin-commonjs";
import nodeResolve from "@rollup/plugin-node-resolve";
import terser from "@rollup/plugin-terser";
import typescript from "@rollup/plugin-typescript";
import fs from "node:fs";
import path from "node:path";
import url from "node:url";

const isWatching = !!process.env.ROLLUP_WATCH;
const sdPlugin = "com.sven.mikrofonsko-stojalo-pridobivanje-imen.sdPlugin";

function readEnvFile() {
  const values = {};
  const envPath = path.resolve(".env");

  if (!fs.existsSync(envPath)) return values;

  for (const line of fs.readFileSync(envPath, "utf8").split(/\r?\n/)) {
    const match = line.match(
      /^\s*([A-Z_][A-Z0-9_]*)\s*=\s*["']?(.*?)["']?\s*$/i,
    );
    if (match) values[match[1]] = match[2];
  }

  return values;
}

const env = { ...process.env, ...readEnvFile() };
const requiredEnv = ["BASE_URL", "USERNAME", "PASSWORD"];
const missingEnv = requiredEnv.filter((name) => !env[name]);

if (missingEnv.length > 0) {
  throw new Error(`Missing plugin configuration: ${missingEnv.join(", ")}`);
}

const injectEnv = {
  name: "inject-plugin-env",
  transform(code) {
    return code.replace(
      /process\.env\.(BASE_URL|USERNAME|PASSWORD)/g,
      (_, name) => JSON.stringify(env[name]),
    );
  },
};

/**
 * @type {import('rollup').RollupOptions}
 */
const config = {
  input: "src/plugin.ts",
  output: {
    file: `${sdPlugin}/bin/plugin.js`,
    sourcemap: isWatching,
    sourcemapPathTransform: (relativeSourcePath, sourcemapPath) => {
      return url.pathToFileURL(
        path.resolve(path.dirname(sourcemapPath), relativeSourcePath),
      ).href;
    },
  },
  plugins: [
    injectEnv,
    {
      name: "watch-externals",
      buildStart: function () {
        this.addWatchFile(`${sdPlugin}/manifest.json`);
      },
    },
    typescript({
      mapRoot: isWatching ? "./" : undefined,
    }),
    nodeResolve({
      browser: false,
      exportConditions: ["node"],
      preferBuiltins: true,
    }),
    commonjs(),
    !isWatching && terser(),
    {
      name: "emit-module-package-file",
      generateBundle() {
        this.emitFile({
          fileName: "package.json",
          source: `{ "type": "module" }`,
          type: "asset",
        });
      },
    },
  ],
};

export default config;
