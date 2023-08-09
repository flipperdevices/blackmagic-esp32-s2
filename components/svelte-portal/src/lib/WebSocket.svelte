<script>
    import { api } from "./Api.svelte";
    import { onMount, onDestroy } from "svelte";

    export let receive = () => {};

    // const tag = "uart-terminal";

    function cleanup_server() {
        let url = api.server;
        if (url == "") {
            url = window.location.host;
        }
        url = url.replaceAll("http://", "");
        url = url.replaceAll("https://", "");
        return url;
    }

    let gateway = `ws://${cleanup_server()}/api/v1/uart/websocket`;
    let websocket;

    function on_open(event) {
        console.log("Connection opened");
    }

    function on_close(event) {
        console.log("Connection closed");
        setTimeout(init, 1000);
    }

    function process(array) {
        receive(array);
    }

    function on_message(event) {
        let data = event.data;

        var fileReader = new FileReader();
        fileReader.onload = function (event) {
            process(new Uint8Array(event.target.result));
        };

        if (data instanceof Blob) {
            fileReader.readAsArrayBuffer(data);
        }
    }

    function init() {
        console.log("Trying to open a WebSocket connection...");
        websocket = new WebSocket(gateway);
        websocket.onopen = on_open;
        websocket.onclose = on_close;
        websocket.onmessage = on_message;
    }

    function destroy() {
        websocket.onclose = function () {}; // disable onclose handler first
        websocket.close();
    }

    onMount(() => {
        init();
    });

    onDestroy(() => {
        destroy();
    });
</script>
