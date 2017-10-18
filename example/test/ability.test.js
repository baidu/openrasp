/**
 * @file
 */
'use strict';
/* eslint-env mocha */
/* globals Context */
const expect = require('chai').expect;
describe('自定义测试', function () {
    describe('SQL注入测试', function () {
        it('union 注入', function () {
            var results = RASP.check('sql', {
                server: 'mysql',
                query: `select name, email from users where id = 1002 and 1=2
                union select table_name, table_schema from information_schema.tables`
            }, new Context());
            expect(results).to.be.a('array');
            results.forEach(result => {
                expect(result).to.have.property('action').to.equal('block');
                expect(result).to.have.property('message').to.be.a('string');
            });
        });
    });
});
