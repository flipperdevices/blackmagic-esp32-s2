<script>
    import { onMount } from "svelte";
    import parseTerminal from "./terminal.js";

    let bytes = [];
    export function push(data) {
        bytes.push(...data);
        process_bytes();
    }

    let ready = {
        lines: [],
        last: "",
    };

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

    onMount(() => {});

    const scrollToBottom = (node) => {
        const scroll = () =>
            node.scroll({
                top: node.scrollHeight,
                behavior: "instant",
            });
        scroll();

        return { update: scroll };
    };
</script>

<div class="terminal" use:scrollToBottom={ready}>
    {#each ready.lines as line}
        <div class="line">{@html line}</div>
    {/each}
    {#if ready.last}
        <div class="line">{@html ready.last}<span class="cursor">_</span></div>
    {/if}
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

    .terminal {
        height: calc(100vh - 20px * 4.5 - 1em);
        overflow: scroll;
        -moz-user-select: text;
        -o-user-select: text;
        -khtml-user-select: text;
        -webkit-user-select: text;
        -ms-user-select: text;
        user-select: text;
        font-size: 18px;
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
