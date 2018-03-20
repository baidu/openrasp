/**
 * @file
 */
'use strict';

const CheckPointCommand = class {
    constructor(data) {
        this.command = data.command;
    }

    static get name() {
        return 'command';
    }
};
Object.defineProperty(global, 'CheckPointCommand', {
    value: CheckPointCommand,
    enumerable: true
});

const CheckPointDirectory = class {
    constructor(data) {
        this.path = data.path;
        this.realpath = data.realpath;
    }

    static get name() {
        return 'directory';
    }
};
Object.defineProperty(global, 'CheckPointDirectory', {
    value: CheckPointDirectory,
    enumerable: true
});

const CheckPointFileUpload = class {
    constructor(data) {
        this.filename = data.filename;
        this.content = data.content;
    }

    static get name() {
        return 'fileUpload';
    }
};
Object.defineProperty(global, 'CheckPointFileUpload', {
    value: CheckPointFileUpload,
    enumerable: true
});

const CheckPointReadFile = class {
    constructor(data) {
        this.path = data.path;
        this.realpath = data.realpath;
    }

    static get name() {
        return 'readFile';
    }
};
Object.defineProperty(global, 'CheckPointReadFile', {
    value: CheckPointReadFile,
    enumerable: true
});

const CheckPointRequest = class {
    constructor(data) {
        this.request = data.request;
    }

    static get name() {
        return 'request';
    }
};
Object.defineProperty(global, 'CheckPointRequest', {
    value: CheckPointRequest,
    enumerable: true
});

const CheckPointSQL = class {
    constructor(data) {
        this.query = data.query;
        this.server = data.server;
    }

    static get name() {
        return 'sql';
    }
};
Object.defineProperty(global, 'CheckPointSQL', {
    value: CheckPointSQL,
    enumerable: true
});

const CheckPointWriteFile = class {
    constructor(data) {
        this.name = data.name;
        this.realpath = data.realpath;
        this.content = data.content;
    }

    static get name() {
        return 'writeFile';
    }
};
Object.defineProperty(global, 'CheckPointWriteFile', {
    value: CheckPointWriteFile,
    enumerable: true
});

const CheckPointXXE = class {
    constructor(data) {
        this.entity = data.entity;
    }

    static get name() {
        return 'xxe';
    }
};
Object.defineProperty(global, 'CheckPointXXE', {
    value: CheckPointXXE,
    enumerable: true
});

const CheckPointOGNL = class {
    constructor(data) {
        this.expression = data.expression;
    }

    static get name() {
        return 'ognl';
    }
};
Object.defineProperty(global, 'CheckPointOGNL', {
    value: CheckPointOGNL,
    enumerable: true
});

const CheckPointDeserialization = class {
    constructor(data) {
        this.clazz = data.clazz;
    }

    static get name() {
        return 'deserialization';
    }
};
Object.defineProperty(global, 'CheckPointDeserialization', {
    value: CheckPointDeserialization,
    enumerable: true
});

const CheckPointReflection = class {
    constructor(data) {
        this.clazz = data.clazz;
    }

    static get name() {
        return 'reflection';
    }
};
Object.defineProperty(global, 'CheckPointReflection', {
    value: CheckPointReflection,
    enumerable: true
});

const CheckPointWebdav = class {
    constructor(data) {
        this.clazz = data.clazz;
    }

    static get name() {
        return 'webdav';
    }
};
Object.defineProperty(global, 'CheckPointWebdav', {
    value: CheckPointWebdav,
    enumerable: true
});

const CheckPointSSRF = class {
    constructor(data) {
        this.clazz = data.clazz;
    }

    static get name() {
        return 'ssrf';
    }
};
Object.defineProperty(global, 'CheckPointSSRF', {
    value: CheckPointSSRF,
    enumerable: true
});

const CheckPointInclude = class {
    constructor(data) {
        this.clazz = data.clazz;
    }

    static get name() {
        return 'include';
    }
};
Object.defineProperty(global, 'CheckPointInclude', {
    value: CheckPointInclude,
    enumerable: true
});
