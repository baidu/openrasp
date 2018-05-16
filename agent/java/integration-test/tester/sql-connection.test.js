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
const CONF_FILE = SERVER_HOME + '/rasp/conf/rasp.properties';
const POLICY_ALARM_FILE = SERVER_HOME + '/rasp/logs/policy_alarm/policy_alarm.log';
const RASP_LOG_FILE = SERVER_HOME + '/rasp/logs/rasp/rasp.log';
const watchFileOptions = {
    persistent: false,
    interval: 400
};
chai.expect();
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
    after(function () {
        fs.writeFileSync(CONF_FILE, '');
    });
    beforeEach(function () {
        fs.writeFileSync(CONF_FILE, '');
        fs.writeFileSync(POLICY_ALARM_FILE, '');
        fs.writeFileSync(RASP_LOG_FILE, '');
    });
    afterEach(function () {
        fs.unwatchFile(POLICY_ALARM_FILE);
        fs.unwatchFile(RASP_LOG_FILE);
    });
    it('should not log and not block when security.enforce_policy=false and request_url=http://127.0.0.1:8080/app/sql-not-connectable.jsp', function (done) {
        let timestamp = Date.now();
        let resData;
        fs.watchFile(POLICY_ALARM_FILE, watchFileOptions, () => {
            let data = fs.readFileSync(POLICY_ALARM_FILE, {
                encoding: 'utf8'
            });
            data.should.equal(timestamp+'')
            resData.should.not.contains('blocked')
            done();
        });
        axios.get('sql-not-connectable.jsp').then(function(response){
            resData = response.data
            fs.writeFileSync(POLICY_ALARM_FILE, timestamp);
        })
    });
    it('should block when security.enforce_policy=true and request_url=http://127.0.0.1:8080/app/sql-connectable.jsp', function (done) {
        let resData;
        fs.watchFile(RASP_LOG_FILE, watchFileOptions, () => {
            let data = fs.readFileSync(RASP_LOG_FILE, {
                encoding: 'utf8'
            });
            if(data.indexOf('configuration')>=0){
                axios.get('sql-connectable.jsp').then(function(response){
                    resData = response.data
                    resData.should.contains('blocked')
                    done()
                })
            }
        });
        fs.writeFileSync(CONF_FILE,'\nsecurity.enforce_policy=true');
    });
    it('should block when security.enforce_policy=false and request_url=http://127.0.0.1:8080/app/sql-not-connectable.jsp and ', function (done) {
        fs.watchFile(RASP_LOG_FILE, watchFileOptions, () => {
            let data = fs.readFileSync(RASP_LOG_FILE, {
                encoding: 'utf8'
            });
            if(data.indexOf('400')>=0){
                axios.get('sql-not-connectable.jsp').then(function(response){
                    response.data.should.contains('blocked')
                    response.status.should.equal(400)
                    done();
                })
            }
        });
        fs.writeFileSync(CONF_FILE,'\nsecurity.enforce_policy=true\nblock.status_code=400');
    });
});
