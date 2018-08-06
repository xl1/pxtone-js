const BUFFER_SIZE = 2048;
const NUMBER_OF_CHANNELS = 2;
const BUFFER_BYTES = BUFFER_SIZE * NUMBER_OF_CHANNELS * 2;
const BITS_PER_SAMPLE = 16;
const SAMPLING_RATE = new AudioContext().sampleRate;

class PxtoneAudio {
    isRunning = false;

    constructor(pxtn) {
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

    // load file
    error = Module.pxtnLoadPtcop(pxtn, 'sample.ptcop');
    if (error && error.value != 0) {
        alert(Module.pxtnErrorToString(error));
        return;
    }

    console.log(`total sample: ${pxtn.totalSample}`);

    const prep = {
        flags: 1, // loop,
        startPosition: 0.0,
        masterVolume: 0.8,
    };
    if (!Module.pxtnServiceMooPreparation(pxtn, prep)) {
        alert('failed to prepare');
        return;
    }

    const audio = new PxtoneAudio(pxtn);
    const button = document.createElement('button');
    button.append('start');
    button.addEventListener('click', () => {
        if (audio.isRunning) {
            audio.pause();
        } else {
            audio.start();
        }
    }, false);
    document.body.append(button);
}

this.Module = {
    onRuntimeInitialized: init
};
