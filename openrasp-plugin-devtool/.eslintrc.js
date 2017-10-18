'use strict'
module.exports = {
    'env': {
        'es6': true,
        'commonjs': true,
        'node': true
    },
    'extends': 'eslint:recommended',
    'rules': {
        'indent': ['error', 4],
        'linebreak-style': 'off',
        'quotes': 'off',
        'semi': ['error', 'always'],
        'no-unused-vars': 'off',
        'no-console': 'off',
        'space-before-blocks': ['error', 'always'],
        'space-infix-ops': 'error',
        'space-before-function-paren': ['error', {
            'anonymous': 'always',
            'named': 'never'
        }],
        'arrow-spacing': 'error',
        'comma-spacing': ['error', {'before': false, 'after': true}],
        'keyword-spacing': ['error', {'before': true, 'after': true}],
        'key-spacing': ['error', {'beforeColon': false, 'afterColon': true}]
    },
    'globals':{
        'RASP': true,
        'Attack': true,
        'PluginError': true,
        'Context': true
    }
}
