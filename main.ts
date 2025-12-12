//% weight=100 color=#ff6600 icon="\uf130"
namespace voiceRadio {

    // C++ bindings
    //% shim=voiceRadio::record
    function nativeRecord(): Buffer { return Buffer.create(0) }

    //% shim=voiceRadio::play
    function nativePlay(buf: Buffer): void { }

    // Record 1 second of μ-law compressed audio
    //% block="record voice"
    export function record(): Buffer {
        return nativeRecord()
    }

    // Play μ-law compressed audio
    //% block="play voice from buffer %buf"
    export function play(buf: Buffer) {
        nativePlay(buf)
    }

    // Send via radio in 32-byte chunks
    //% block="send voice buffer %buf"
    export function send(buf: Buffer) {
        const chunkSize = 32
        for (let i = 0; i < buf.length; i += chunkSize) {
            const slice = buf.slice(i, chunkSize)
            radio.sendBuffer(slice)
            basic.pause(5)
        }
        radio.sendString("END")
    }

    // Receive all chunks and then auto-play
    //% block="receive voice"
    export function receive(): Buffer {
        let parts: Buffer[] = []
        radio.onReceivedBuffer(function (b) {
            parts.push(b)
        })
        radio.onReceivedString(function (s) {
            if (s == "END") {
                let total = 0
                for (let p of parts) total += p.length

                let out = Buffer.create(total)
                let offset = 0
                for (let p of parts) {
                    out.write(offset, p)
                    offset += p.length
                }
                voiceRadio.play(out)
            }
        })
        return null
    }
}
