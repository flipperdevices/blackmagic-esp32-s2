<script>
    import { onMount } from "svelte";
    import parseTerminal from "./terminal.js";
    import Button from "./Button.svelte";
    import Popup from "./Popup.svelte";
    import Spinner from "./Spinner.svelte";
    import SpinnerBig from "./SpinnerBig.svelte";
    import { api } from "../lib/Api.svelte";
    import Grid from "./Grid.svelte";
    import Value from "./Value.svelte";
    import Input from "./Input.svelte";
    import StringView from "stringview/StringView";
    import Select from "./Select.svelte";

    let bytes = new Uint8Array(0);

    function cat_arrays(a, b) {
        var c = new a.constructor(a.length + b.length);
        c.set(a, 0);
        c.set(b, a.length);
        return c;
    }

    export const push = (data) => {
        bytes = cat_arrays(bytes, data);
        process_bytes();
    };

    export let on_mount = () => {};
    export let send = () => {};

    let ready = {
        lines: [],
        last: "",
    };

    const line_empty_to_br = (line) => {
        if (line.trim() == "") {
            return "<br>";
        } else {
            return line;
        }
    };

    const process_bytes = () => {
        // convert to DataView
        const data_view = new DataView(
            bytes.buffer,
            bytes.byteOffset,
            bytes.byteLength
        );

        const encoding = "ASCII";
        const eol = "\n";
        const eol_code = eol.charCodeAt(0);

        // find last EOL
        const last_eol = bytes.lastIndexOf(eol_code);

        if (last_eol != -1) {
            // decode bytes from 0 to last_eol
            const decoded = StringView.getString(
                data_view,
                0,
                last_eol,
                encoding
            );

            // split by EOL
            let lines = decoded.split(eol);

            // parse and push lines
            lines = lines.map((line) => parseTerminal(line));
            ready.lines.push(...lines);

            // remove processed bytes
            bytes = bytes.subarray(last_eol + 1);
        }

        // decode last line
        if (bytes.length > 0) {
            const last_string = StringView.getString(
                data_view,
                0,
                bytes.length,
                encoding
            );
            ready.last = parseTerminal(last_string);
        } else {
            ready.last = "";
        }
    };

    onMount(() => {
        on_mount();
    });

    const scrollToBottom = (node) => {
        const scroll = () =>
            node.scroll({
                top: node.scrollHeight,
                behavior: "instant",
            });
        scroll();

        return { update: scroll };
    };

    let popup = {
        text: "",
        self: null,
    };

    let config = {
        popup: null,
        bit_rate: null,
        stop_bits: null,
        parity: null,
        data_bits: null,
    };

    async function config_apply() {
        popup.text = "";
        popup.self.show();
        popup = popup;
        config.popup.close();

        await api
            .post("/api/v1/uart/set_config", {
                bit_rate: parseInt(config.bit_rate.get_value()),
                stop_bits: parseInt(config.stop_bits.get_value()),
                parity: parseInt(config.parity.get_value()),
                data_bits: parseInt(config.data_bits.get_value()),
            })
            .then((json) => {
                if (json.error) {
                    popup.text = json.error;
                } else {
                    popup.text = "Saved!";
                }
            });
    }

    let tx = {
        popup: null,
        data: "",
        eol: "\\r\\n",
    };

    async function uart_send() {
        tx.popup.close();
        let eol = tx.eol.replaceAll("\\r", "\r").replaceAll("\\n", "\n");
        let data = tx.data + eol;

        // split data to chunks of 1k bytes
        let chunks = [];
        while (data.length > 0) {
            chunks.push(data.slice(0, 1024));
            data = data.slice(1024);
        }

        for (let chunk of chunks) {
            send(chunk);
        }
    }
</script>

<div class="terminal-wrapper">
    <div class="terminal selectable" use:scrollToBottom={ready}>
        <div class="line">
            {#each ready.lines as line}
                {@html line}<br />
            {/each}
        </div>
        {#if ready.last}
            <div class="line">
                {@html ready.last}<span class="cursor">_</span>
            </div>
        {/if}
    </div>
    <div class="config">
        <Button value="S" on:click={tx.popup.show} />
        <Button value="#" on:click={config.popup.show} />
    </div>
    <Popup bind:this={config.popup}>
        {#await api.get("/api/v1/uart/get_config", {})}
            <SpinnerBig />
        {:then json}
            <div>UART config</div>
            <Grid>
                <Value name="Rate">
                    <Input
                        type="number"
                        value={json.bit_rate}
                        bind:this={config.bit_rate}
                    />
                </Value>
                <Value name="Stop">
                    <Select
                        bind:this={config.stop_bits}
                        items={[
                            { text: "1", value: "0" },
                            { text: "1.5", value: "1" },
                            { text: "2", value: "2" },
                        ]}
                        value={json.stop_bits.toString()}
                    />
                </Value>
                <Value name="Prty">
                    <Select
                        bind:this={config.parity}
                        items={[
                            { text: "None", value: "0" },
                            { text: "Odd", value: "1" },
                            { text: "Even", value: "2" },
                        ]}
                        value={json.parity.toString()}
                    />
                </Value>
                <Value name="Data">
                    <Select
                        bind:this={config.data_bits}
                        items={[
                            { text: "5", value: "5" },
                            { text: "6", value: "6" },
                            { text: "7", value: "7" },
                            { text: "8", value: "8" },
                        ]}
                        value={json.data_bits.toString()}
                    />
                </Value>
            </Grid>
            <div style="margin-top: 10px; text-align: center;">
                <Button value="Save" on:click={config_apply} />
            </div>
        {:catch error}
            <error>{error.message}</error>
        {/await}
    </Popup>

    <Popup bind:this={popup.self}>
        {#if popup.text != ""}
            {popup.text}
        {:else}
            <Spinner />
        {/if}
    </Popup>

    <Popup bind:this={tx.popup}>
        <Grid>
            <Value name="Data">
                <Input value={tx.data} input={(data) => (tx.data = data)} /><br
                />
            </Value>
            <Value name="EOL">
                <Input value={tx.eol} input={(data) => (tx.eol = data)} />
            </Value>
        </Grid>
        <div style="margin-top: 10px; text-align: center;">
            <Button value="Send" on:click={uart_send} />
        </div>
    </Popup>
</div>

<style>
    @keyframes blink {
        0% {
            opacity: 1;
        }
        49% {
            opacity: 1;
        }
        50% {
            opacity: 0;
        }
        99% {
            opacity: 0;
        }
        100% {
            opacity: 1;
        }
    }

    .cursor {
        animation: blink 1s infinite;
    }

    .line {
        display: block;
    }

    .terminal-wrapper {
        position: relative;
        height: 100%;
    }

    .terminal {
        height: 100%;
        font-size: 18px;
        overflow-y: scroll;
        overflow-x: clip;
        white-space: wrap;
    }

    .config {
        position: absolute;
        top: 0;
        right: 0;
    }

    :global(.terminal.bold) {
        font-weight: bold;
    }

    :global(.terminal.underline) {
        text-decoration: underline;
    }

    :global(.terminal.blink) {
        animation: blink 1s infinite;
    }

    :global(.terminal.invisible) {
        display: none;
    }

    :global(.terminal-wrapper select) {
        width: 100%;
    }
</style>
