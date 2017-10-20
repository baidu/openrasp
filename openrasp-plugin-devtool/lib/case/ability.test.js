/**
 * @file
 */
'use strict';
/* eslint-env mocha */
/* global RASP, Context */

const expect = require('chai').expect;
const fs = require('fs');
const path = require('path');
const program = require('commander');

describe('插件能力测试', function () {
    describe('sql', function () {
        before(function () {
            if (!Array.isArray(RASP.checkPoints.sql) ||
                RASP.checkPoints.sql.length === 0) {
                this.skip();
            }
        });

        describe('安全', function ()  {
            let safe = fs.readFileSync(path.resolve(__dirname, './sql-safe'), 'utf8')
                .replace(/\r\n/g, '\n').split('\n').filter(sql => sql.trim().length !== 0);
            safe.forEach(sql => {
                it(sql, function ()  {
                    let results = RASP.check('sql', {
                        query: sql,
                        server: 'mysql'
                    }, new Context());
                    expect(results).to.be.a('array');
                    results.forEach(result => {
                        expect(result).to.be.a('object');
                        expect(result.action).to.be.a('string').but.not.equal('block');
                    });
                });
            });
        });

        describe('不安全', function ()  {
            let unsafe = fs.readFileSync(path.resolve(__dirname, './sql-unsafe'), 'utf8')
                .replace(/\r\n/g, '\n').split('\n').filter(sql => sql.trim().length !== 0);
            unsafe.forEach(sql => {
                it(sql, function () {
                    let results = RASP.check('sql', {
                        query: sql,
                        server: 'mysql'
                    }, new Context());
                    expect(results).to.be.a('array');
                    results.forEach(result => {
                        expect(result).to.be.a('object');
                        expect(result.action).to.be.a('string').and.equal('block');
                    });
                });
            });
        });
    });

    describe('xxe', function () {
        before(function () {
            if (!Array.isArray(RASP.checkPoints.xxe) ||
                RASP.checkPoints.xxe.length === 0) {
                this.skip();
            }
        });

        describe('安全', function () {});
        describe('不安全', function () {});
    });

    describe('directory', function () {
        before(function () {
            if (!Array.isArray(RASP.checkPoints.directory) ||
                RASP.checkPoints.directory.length === 0) {
                this.skip();
            }
        });

        describe('安全', function () {});
        describe('不安全', function () {});
    });

    describe('request', function () {
        before(function () {
            if (!Array.isArray(RASP.checkPoints.request) ||
                RASP.checkPoints.request.length === 0) {
                this.skip();
            }
        });

        describe('安全', function () {});
        describe('不安全', function () {});
    });

    describe('readFile', function () {
        before(function () {
            if (!Array.isArray(RASP.checkPoints.readFile) ||
                RASP.checkPoints.readFile.length === 0) {
                this.skip();
            }
        });

        describe('安全', function () {});
        describe('不安全', function () {});
    });

    describe('writeFile', function () {
        before(function () {
            if (!Array.isArray(RASP.checkPoints.writeFile) ||
                RASP.checkPoints.writeFile.length === 0) {
                this.skip();
            }
        });

        describe('安全', function () {});
        describe('不安全', function () {});
    });

    describe('fileUpload', function () {
        before(function () {
            if (!Array.isArray(RASP.checkPoints.fileUpload) ||
                RASP.checkPoints.fileUpload.length === 0) {
                this.skip();
            }
        });

        describe('安全', function () {});
        describe('不安全', function () {});
    });

    describe('command', function () {
        before(function () {
            if (!Array.isArray(RASP.checkPoints.command) ||
                RASP.checkPoints.command.length === 0) {
                this.skip();
            }
        });

        describe('安全', function () {});
        describe('不安全', function () {});
    });
});
