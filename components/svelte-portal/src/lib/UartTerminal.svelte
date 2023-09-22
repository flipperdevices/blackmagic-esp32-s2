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

    let bytes = [];
    export function push(data) {
        bytes.push(...data);
        process_bytes();
    }

    let ready = {
        lines: [],
        last: "",
    };

    export let on_mount = () => {};

    export let send;

    function process_bytes() {
        let decoded = new TextDecoder().decode(new Uint8Array(bytes));
        let last_line_complete =
            decoded.lastIndexOf("\n") == decoded.length - 1;

        let lines = decoded.split("\n");

        bytes = [];
        if (!last_line_complete) {
            ready.last = lines.pop();
            bytes.push(...new TextEncoder().encode(ready.last));
        } else {
            ready.last = "";
        }

        lines = lines.map((line) => parseTerminal(line));
        ready.last = parseTerminal(ready.last);

        ready.lines.push(...lines);
        ready = ready;
    }

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
        send("s" + data);
    }
</script>

<div class="terminal-wrapper">
    <div class="terminal selectable" use:scrollToBottom={ready}>
        {#each ready.lines as line}
            <div class="line">{@html line}</div>
        {/each}
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
                    <Input
                        type="number"
                        value={json.stop_bits}
                        bind:this={config.stop_bits}
                    />
                </Value>
                <Value name="Prty">
                    <Input
                        type="number"
                        value={json.parity}
                        bind:this={config.parity}
                    />
                </Value>
                <Value name="Data">
                    <Input
                        type="number"
                        value={json.data_bits}
                        bind:this={config.data_bits}
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
    }

    .terminal {
        height: calc(100vh - 20px * 4.5 - 1em);
        overflow: scroll;
        font-size: 18px;
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
</style>
