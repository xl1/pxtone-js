// global: Module

Module['preRun'] = function () {
    FS.createPreloadedFile('/', 'sample.ptcop', '/sample data/sample.ptcop', true, false);
};
