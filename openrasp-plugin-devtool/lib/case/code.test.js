/**
 * @file
 */
'use strict';
/* eslint-env mocha */

const fs = require('fs');
const vm = require('vm');
const path = require('path');
const program = require('commander');
const CLIEngine = require('eslint').CLIEngine;
let cli = new CLIEngine({
    configFile: path.resolve(__dirname, 'eslintconfig.js'),
    useEslintrc: false
});

describe('插件代码检查', function () {
    before(function () {
        this.filename = program.args[0];
        this.filepath = path.resolve(process.cwd(), this.filename);
    });

    it('代码规范', function () {
        this.slow(500);
        let report = cli.executeOnFiles([this.filepath]);
        let formatter = cli.getFormatter();

        if (report) {
            if (report.errorCount > 0) {
                throw new Error(formatter(report.results));
            } else if (report.warningCount > 0) {
                console.log(formatter(report.results));
            }
        }
    });

    it('模拟环境', function () {
        let sandbox = {console};
        sandbox.global = sandbox;
        vm.createContext(sandbox);

        let checkpoint = path.resolve(__dirname, '..', 'environment', 'checkpoint.js');
        let context = path.resolve(__dirname, '..', 'environment', 'context.js');
        let error = path.resolve(__dirname, '..', 'environment', 'error.js');
        let rasp = path.resolve(__dirname, '..', 'environment', 'rasp.js');

        vm.runInContext(`(function(){ ${fs.readFileSync(checkpoint, 'utf8')} })()`, sandbox, {
            filename: 'checkpoint'
        });
        vm.runInContext(`(function(){ ${fs.readFileSync(context, 'utf8')} })()`, sandbox, {
            filename: 'context'
        });
        vm.runInContext(`(function(){ ${fs.readFileSync(error, 'utf8')} })()`, sandbox, {
            filename: 'error'
        });
        vm.runInContext(`(function(){ ${fs.readFileSync(rasp, 'utf8')} })()`, sandbox, {
            filename: 'rasp'
        });
        vm.runInContext(`(function(){ ${fs.readFileSync(this.filepath, 'utf8')} })()`, sandbox, {
            filename: this.filename
        });
    });
});
