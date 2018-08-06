const BUFFER_SIZE = 2048;
const NUMBER_OF_CHANNELS = 2;
const BUFFER_BYTES = BUFFER_SIZE * NUMBER_OF_CHANNELS * 2;
const BITS_PER_SAMPLE = 16;
const SAMPLING_RATE = new AudioContext().sampleRate;

function readBlobAsArrayBufferAsync(blob) {
    return new Promise((resolve, reject) => {
        const reader = new FileReader();
        reader.onload = _ => resolve(reader.result);
        reader.onerror = reject;
        reader.readAsArrayBuffer(blob);
    });
}

class PxtoneAudio {
    constructor(pxtn) {
        this.pxtn = pxtn;
        this.isRunning = false;
        this.ctx = new AudioContext();
        this.procNode = this.ctx.createScriptProcessor(BUFFER_SIZE, 0, NUMBER_OF_CHANNELS);

        const buffer = Module._malloc(BUFFER_BYTES);

        this.procNode.addEventListener('audioprocess', ({ outputBuffer }) => {
            const result = Module.ccall('pxtnServiceMoo', 'bool', ['number', 'number', 'number'], [pxtn.$$.ptr, buffer, BUFFER_BYTES]);
            const input = new Int16Array(Module.HEAP16.buffer, buffer, BUFFER_SIZE * NUMBER_OF_CHANNELS);

            for (let ch = 0; ch < NUMBER_OF_CHANNELS; ch++) {
                const output = outputBuffer.getChannelData(ch);
                for (let i = 0; i < BUFFER_SIZE; i++) {
                    output[i] = input[i * NUMBER_OF_CHANNELS + ch] / 0x8000;
                }
            }
        });
    }

    async loadFileAsync(blob) {
        const fileBuffer = await readBlobAsArrayBufferAsync(blob);
        const buffer = Module._malloc(fileBuffer.byteLength);
        Module.HEAPU8.set(new Uint8Array(fileBuffer), buffer);

        const err = Module.ccall('pxtnServiceLoad', 'number', ['number', 'number', 'number'], [this.pxtn.$$.ptr, buffer, fileBuffer.byteLength]);
        Module._free(buffer);

        if (err) throw new Error(Module.pxtnErrorToString({ value: err }));

        const prep = {
            flags: 1, // loop,
            startPosition: 0.0,
            masterVolume: 0.8,
        };
        if (!Module.pxtnServiceMooPreparation(this.pxtn, prep)) {
            throw new Error('failed to prepare');
        }
    }

    start() {
        this.isRunning = true;
        this.procNode.connect(this.ctx.destination);
    }
    pause() {
        this.isRunning = false;
        this.procNode.disconnect();
    }
}

function init() {
    const pxtn = new Module.PxtnService();

    let error = pxtn.init();
    if (error && error.value != 0) {
        alert(Module.pxtnErrorToString(error));
        return;
    }

    if (!pxtn.setDestinationQuality(NUMBER_OF_CHANNELS, SAMPLING_RATE)) {
        alert('failed to set destination quality');
        return;
    }

    const audio = new PxtoneAudio(pxtn);
    const input = document.createElement('input');
    input.type = 'file';
    input.accept = '.ptcop,.pttune';
    input.addEventListener('change', _ => {
        audio.pause();
        const file = input.files[0];
        if (file) {
            audio.loadFileAsync(file)
                .then(() => audio.start())
                .catch(err => alert(err.message || err));
        }
    }, false);
    document.body.append(input);
}

this.Module = {
    onRuntimeInitialized: init
};
