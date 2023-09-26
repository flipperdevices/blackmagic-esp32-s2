<script>
    import { api } from "./Api.svelte";
    import { onMount, onDestroy } from "svelte";

    export let receive = () => {};
    export const send = send_data;

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

    function send_data(data) {
        websocket.send(data);
    }

    function on_open(event) {}

    function on_close(event) {
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
        websocket = new WebSocket(gateway);
        websocket.onopen = on_open;
        websocket.onclose = on_close;
        websocket.onmessage = on_message;
    }

    function destroy() {
        websocket.onclose = function () {};
        websocket.close();
    }

    onMount(() => {
        init();
    });

    onDestroy(() => {
        destroy();
    });
</script>
