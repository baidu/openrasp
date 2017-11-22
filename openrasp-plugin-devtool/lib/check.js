/**
 * @file
 */
'use strict';
/* eslint-env node */
const fs = require('fs');
const path = require('path');
const program = require('commander');
const Mocha = require('mocha');
require('./buildenv');

program.usage('<file>')
    .option('-t, --test-dir <dir>', 'specify a custom test cases directory')
    .parse(process.argv);

if (program.args.length < 1) {
    return program.help();
}

let filename = program.args[0];
let filepath = path.resolve(process.cwd(), filename);
require(filepath);

let mocha = new Mocha({
    bail: true,
    useColors: true,
    slow: 20,
    reporter: 'list'
});

mocha.addFile(
    path.join(path.resolve(__dirname, 'case', 'code.test.js'))
);
mocha.addFile(
    path.join(path.resolve(__dirname, 'case', 'ability.test.js'))
);

if (program.testDir) {
    fs.readdirSync(path.resolve(process.cwd(), program.testDir))
        .filter(file => file.substr(-8) === '.test.js')
        .forEach(file => {
            mocha.addFile(
                path.join(path.resolve(process.cwd(), program.testDir, file))
            );
        });
}

mocha.run(failures => {
    process.on('exit', () => {
        process.exit(failures);
    });
});
