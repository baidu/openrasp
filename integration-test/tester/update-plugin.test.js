/**
 * @file test
 */
/* eslint-env mocha */
'use strict';
const chai = require('chai').use(require('chai-as-promised'));
const axios = require('axios');
const fs = require('fs');
const timeout = ms => new Promise(resolve => setTimeout(resolve, ms));
const SERVER_HOME = process.env['SERVER_HOME'];
const PLUGIN_LOG = SERVER_HOME + '/rasp/logs/plugin/plugin.log';
const watchFileOptions = {
    persistent: false,
    interval: 200
};

chai.should();
axios.defaults.headers.common['Test-Test'] = 'Test-Test';
axios.defaults.validateStatus = status => status !== undefined;

describe(process.env['SERVER'] || 'server', function () {
    before(function () {
        this.timeout(1000 * 60 * 2);
        axios.defaults.baseURL = 'http://127.0.0.1:8080/app';
        let chain = Promise.reject();
        let request = () => timeout(2000).then(() => axios.get(''));
        for (let i = 0; i < 30; i++) {
            chain = chain
                .catch(() => {
                    process.stdout.write('.');
                    return request();
                });
        }
        chain = chain
            .then(() => console.log());
        process.stdout.write('\tconnecting to server');
        return chain;
    });
    beforeEach(function () {
        fs.writeFileSync(PLUGIN_LOG, '');
    });
    afterEach(function () {
        fs.unwatchFile(PLUGIN_LOG);
    });
    it('should update plugins after creating test.js', function (done) {
        let timestamp = Date.now();
        fs.watchFile(PLUGIN_LOG, watchFileOptions, () => {
            let data = fs.readFileSync(PLUGIN_LOG, {
                encoding: 'utf8'
            });
            data.should.contains(timestamp);
            done();
        });
        fs.writeFileSync(SERVER_HOME + '/rasp/plugins/test.js', `\nconsole.log(${timestamp})`);
    });
    it('should update plugins after modifing plugin.js', function (done) {
        let timestamp = Date.now();
        fs.watchFile(PLUGIN_LOG, watchFileOptions, () => {
            let data = fs.readFileSync(PLUGIN_LOG, {
                encoding: 'utf8'
            });
            data.should.contains(timestamp);
            done();
        });
        fs.appendFileSync(SERVER_HOME + '/rasp/plugins/plugin.js', `\nconsole.log(${timestamp})`);
    });
    it('should update plugins after deleting test.js', function (done) {
        fs.watchFile(PLUGIN_LOG, watchFileOptions, () => {
            let data = fs.readFileSync(PLUGIN_LOG, {
                encoding: 'utf8'
            });
            done();
        });
        fs.unlinkSync(SERVER_HOME + '/rasp/plugins/test.js');
    });
    it('should not update plugins after creating or modifing or deleting dir/test.js', function (done) {
        let error = undefined;
        setTimeout(() => {
            done(error);
        }, 2000);
        fs.watchFile(PLUGIN_LOG, watchFileOptions, () => {
            error = new Error('plugins updated');
        });
        fs.mkdirSync(SERVER_HOME + '/rasp/plugins/dir');
        fs.writeFileSync(SERVER_HOME + '/rasp/plugins/dir/test.js', '\nconsole.log()');
        fs.appendFileSync(SERVER_HOME + '/rasp/plugins/dir/test.js', '\nconsole.log()');
        fs.unlinkSync(SERVER_HOME + '/rasp/plugins/dir/test.js');
        fs.rmdirSync(SERVER_HOME + '/rasp/plugins/dir');
    });
    it('should not update plugins after creating or modifing or deleting plugin.txt', function (done) {
        let error = undefined;
        setTimeout(() => {
            done(error);
        }, 2000);
        fs.watchFile(PLUGIN_LOG, watchFileOptions, () => {
            error = new Error('plugins updated');
        });
        fs.writeFileSync(SERVER_HOME + '/rasp/plugins/plugin.txt', 'test');
        fs.appendFileSync(SERVER_HOME + '/rasp/plugins/plugin.txt', 'test');
        fs.unlinkSync(SERVER_HOME + '/rasp/plugins/plugin.txt');
    });
});