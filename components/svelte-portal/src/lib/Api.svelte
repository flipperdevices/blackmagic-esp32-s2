<script context="module">
    let server = "";

    if (development_mode) {
        server = "http://192.168.0.18";
    }

    export const api = {
        server: server,
        dev_mode: development_mode,
        post: async function (api, data) {
            const res = await fetch(this.server + api, {
                method: "POST",
                body: JSON.stringify(data),
            });

            const json = await res.json();
            return json;
        },
        get: async function (api) {
            const res = await fetch(this.server + api, {
                method: "GET",
            });

            const json = await res.json();
            return json;
        },
    };
</script>
