```
  "name": "DharmveerDal",
  "version": "0.0.1",
  "private": true,
```








```
# Updated Dependencies
The following changes were made to the dependencies:
- Updated all dependencies in `package.json` to their latest versions.
- Installed dependencies using `npm install --legacy-peer-deps` to resolve conflicts.

# Deployment Instructions
To deploy the app on GitHub Pages, run the following command:
npm install gh-pages --save-dev --legacy-peer-deps

Add the following scripts to your `package.json`:
```json
"scripts": {
  "predeploy": "npm run build",
  "deploy": "gh-pages -d build"
}
```
After adding the scripts, you can deploy the app by running:
npm run deploy
⠸^C
~/.../main/irbox.github.io $ ls
App.js   README.md  android   babel.config.js  ios             metro.config.js  src            yarn.lock
Gemfile  __tests__  app.json  index.js         jest.config.js  package.json     tsconfig.json
~/.../main/irbox.github.io $ nano package.json
~/.../main/irbox.github.io $ npm install gh-pages --save-dev --legacy-peer-deps
npm error code EJSONPARSE
npm error path /storage/emulated/0/Download/yo/main/irbox.github.io/package.json
npm error JSON.parse Expected ',' or '}' after property value in JSON at position 160 (line 8 column 5) while parsing near "...\"npm run build\"\n    \"deploy\": \"gh-pages ..."
npm error JSON.parse Failed to parse JSON data.
npm error JSON.parse Note: package.json must be actual JSON, not just JavaScript.
npm error A complete log of this run can be found in: /data/data/com.termux/files/home/.npm/_logs/2025-03-01T21_19_11_519Z-debug-0.log
~/.../main/irbox.github.io $ nano package.json
~/.../main/irbox.github.io $ npm install gh-pages --save-dev --legacy-peer-deps
npm warn deprecated stable@0.1.8: Modern JS already guarantees Array#sort() is a stable sort, so this library is deprecated. See the compatibility table on MDN: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Array/sort#browser_compatibility
npm warn deprecated @babel/plugin-proposal-optional-chaining@7.21.0: This proposal has been merged to the ECMAScript standard and thus this plugin is no longer maintained. Please use @babel/plugin-transform-optional-chaining instead.
npm warn deprecated @babel/plugin-proposal-nullish-coalescing-operator@7.18.6: This proposal has been merged to the ECMAScript standard and thus this plugin is no longer maintained. Please use @babel/plugin-transform-nullish-coalescing-operator instead.
npm warn deprecated @babel/plugin-proposal-class-properties@7.18.6: This proposal has been merged to the ECMAScript standard and thus this plugin is no longer maintained. Please use @babel/plugin-transform-class-properties instead.
npm warn deprecated @babel/plugin-proposal-optional-catch-binding@7.18.6: This proposal has been merged to the ECMAScript standard and thus this plugin is no longer maintained. Please use @babel/plugin-transform-optional-catch-binding instead.
npm warn deprecated @babel/plugin-proposal-numeric-separator@7.18.6: This proposal has been merged to the ECMAScript standard and thus this plugin is no longer maintained. Please use @babel/plugin-transform-numeric-separator instead.
npm warn deprecated @babel/plugin-proposal-async-generator-functions@7.20.7: This proposal has been merged to the ECMAScript standard and thus this plugin is no longer maintained. Please use @babel/plugin-transform-async-generator-functions instead.
npm warn deprecated @babel/plugin-proposal-object-rest-spread@7.20.7: This proposal has been merged to the ECMAScript standard and thus this plugin is no longer maintained. Please use @babel/plugin-transform-object-rest-spread instead.
npm warn deprecated inflight@1.0.6: This module is not supported, and leaks memory. Do not use it. Check out lru-cache if you want a good and tested way to coalesce async requests by a key value, which is much more comprehensive and powerful.
npm warn deprecated glob@7.2.3: Glob versions prior to v9 are no longer supported
npm warn deprecated rimraf@3.0.2: Rimraf versions prior to v4 are no longer supported
npm warn deprecated sudo-prompt@9.2.1: Package no longer supported. Contact Support at https://www.npmjs.com/support for more info.
npm warn deprecated @humanwhocodes/config-array@0.11.10: Use @eslint/config-array instead
npm warn deprecated deep-assign@3.0.0: Check out `lodash.merge` or `merge-options` instead.
npm warn deprecated @babel/plugin-proposal-unicode-property-regex@7.18.6: This proposal has been merged to the ECMAScript standard and thus this plugin is no longer maintained. Please use @babel/plugin-transform-unicode-property-regex instead.
npm warn deprecated @types/metro-config@0.76.3: This is a stub types definition. metro-config provides its own type definitions, so you do not need this installed.
npm warn deprecated rimraf@2.6.3: Rimraf versions prior to v4 are no longer supported
npm warn deprecated @humanwhocodes/object-schema@1.2.1: Use @eslint/object-schema instead
npm warn deprecated uglify-es@3.3.9: support for ECMAScript is superseded by `uglify-js` as of v3.13.0
npm warn deprecated react-native-document-picker@9.0.1: the package was renamed, follow migration instructions at https://shorturl.at/QYT4t
npm warn deprecated @react-native-community/async-storage@1.12.1: Async Storage has moved to new organization: https://github.com/react-native-async-storage/async-storage
npm warn deprecated eslint@8.43.0: This version is no longer supported. Please see https://eslint.org/version-support for other options.
npm warn deprecated core-js@1.2.7: core-js@<3.23.3 is no longer maintained and not recommended for usage due to the number of issues. Because of the V8 engine whims, feature detection in old core-js versions could cause a slowdown up to 100x even if nothing is polyfilled. Some versions have web compatibility issues. Please, upgrade your dependencies to the actual version of core-js.
npm warn deprecated shaka-player@2.5.23: Shaka Player < v4.2 is no longer supported.
npm error code EACCES
npm error syscall symlink
npm error path ../@babel/parser/bin/babel-parser.js
npm error dest /storage/emulated/0/Download/yo/main/irbox.github.io/node_modules/.bin/parser
npm error errno -13
npm error Error: EACCES: permission denied, symlink '../@babel/parser/bin/babel-parser.js' -> '/storage/emulated/0/Download/yo/main/irbox.github.io/node_modules/.bin/parser'
npm error     at async symlink (node:internal/fs/promises:1006:10)
npm error     at async Promise.all (index 0)
npm error     at async Promise.all (index 0)
npm error     at async #createBinLinks (/data/data/com.termux/files/usr/lib/node_modules/npm/node_modules/@npmcli/arborist/lib/arborist/rebuild.js:394:5)
npm error     at async Promise.allSettled (index 0)
npm error     at async #linkAllBins (/data/data/com.termux/files/usr/lib/node_modules/npm/node_modules/@npmcli/arborist/lib/arborist/rebuild.js:375:5)
npm error     at async #build (/data/data/com.termux/files/usr/lib/node_modules/npm/node_modules/@npmcli/arborist/lib/arborist/rebuild.js:160:7)
npm error     at async Arborist.rebuild (/data/data/com.termux/files/usr/lib/node_modules/npm/node_modules/@npmcli/arborist/lib/arborist/rebuild.js:62:5)
npm error     at async [reifyPackages] (/data/data/com.termux/files/usr/lib/node_modules/npm/node_modules/@npmcli/arborist/lib/arborist/reify.js:325:11)
npm error     at async Arborist.reify (/data/data/com.termux/files/usr/lib/node_modules/npm/node_modules/@npmcli/arborist/lib/arborist/reify.js:142:5) {
npm error   errno: -13,
npm error   code: 'EACCES',
npm error   syscall: 'symlink',
npm error   path: '../@babel/parser/bin/babel-parser.js',
npm error   dest: '/storage/emulated/0/Download/yo/main/irbox.github.io/node_modules/.bin/parser'
npm error }
npm error
npm error The operation was rejected by your operating system.
npm error It is likely you do not have the permissions to access this file as the current user
npm error
npm error If you believe this might be a permissions issue, please double-check the
npm error permissions of the file and its containing directories, or try running
npm error the command again as root/Administrator.
npm error A complete log of this run can be found in: /data/data/com.termux/files/home/.npm/_logs/2025-03-01T21_20_16_996Z-debug-0.log
~/.../main/irbox.github.io $
```
