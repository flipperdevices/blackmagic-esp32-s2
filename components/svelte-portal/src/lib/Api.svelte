<script context="module">
    let server = "";

    if (development_mode) {
        server = "http://172.30.1.206";
    }

    // export const api_dev_mode = development_mode;

    // export const api_server = server;

    // export async function api_post(api, data) {
    //     const res = await fetch(api_server + api, {
    //         method: "POST",
    //         body: JSON.stringify(data),
    //     });

    //     const json = await res.json();
    //     return json;
    // }

    // export async function api_get(api) {
    //     const res = await fetch(api_server + api, {
    //         method: "GET",
    //     });

    //     const json = await res.json();
    //     return json;
    // }

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
