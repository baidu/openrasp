
/**
 * @file test
 */
/* eslint-env mocha */
'use strict';
const chai = require('chai').use(require('chai-as-promised'));
const axios = require('axios');
const FormData = require('form-data');
const timeout = ms => new Promise(resolve => setTimeout(resolve, ms));

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
    it('fileUpload', function () {
        let form = new FormData();
        form.append('test', new Buffer(10), {
            filename: 'test.txt',
            contentType: 'test/plain',
            knownLength: 10
        });
        return axios.post('fileUpload' + '.jsp?test=a&test=b', form, {
            headers: form.getHeaders()
        }).should.eventually.have.property('data')
            .match(/blocked/);
    });
    let checkPoints = ['command', 'deserialization', 'directory',
        'ognl', 'readFile', 'request', 'writeFile', 'xxe', 'jstlImport', 'sqlite', 'postgresql', 'mysql','http-commonclient','http-httpclient','http-urlconnection'];
    checkPoints.forEach(point => {
        it(point, function () {
            return axios.get(point + '.jsp?test=a&test=b')
                .should.eventually.have.property('data')
                .match(/blocked/);
        });
    });
});
