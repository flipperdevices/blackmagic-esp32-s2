<script>
    import { api } from "../lib/Api.svelte";
    import Input from "../lib/Input.svelte";
    import Spinner from "../lib/Spinner.svelte";
    import SpinnerBig from "../lib/SpinnerBig.svelte";
    import Button from "../lib/Button.svelte";
    import ButtonInline from "../lib/ButtonInline.svelte";
    import Select from "../lib/Select.svelte";
    import Popup from "../lib/Popup.svelte";
    import Value from "../lib/Value.svelte";
    import Grid from "../lib/Grid.svelte";

    let mode_select;
    let usb_mode_select;
    let ap_ssid_input;
    let ap_pass_input;
    let sta_ssid_input;
    let sta_pass_input;
    let hostname_input;
    let popup_select_net;

    let popup = {
        text: "",
        self: null,
    };

    async function reboot_board() {
        api.post("/api/v1/system/reboot", {});
        popup.text = "Rebooted";
        popup.self.show();
    }

    async function save_settings() {
        popup.text = "";
        popup.self.show();
        popup = popup;

        await api
            .post("/api/v1/wifi/set_credentials", {
                wifi_mode: mode_select.get_value(),
                usb_mode: usb_mode_select.get_value(),
                ap_ssid: ap_ssid_input.get_value(),
                ap_pass: ap_pass_input.get_value(),
                sta_ssid: sta_ssid_input.get_value(),
                sta_pass: sta_pass_input.get_value(),
                hostname: hostname_input.get_value(),
            })
            .then((json) => {
                if (json.error) {
                    popup.text = json.error;
                } else {
                    popup.text = "Saved!";
                }
            });
    }
</script>

<Grid>
    {#await api.get("/api/v1/wifi/get_credentials")}
        <Value name="Mode"><Spinner /></Value>
        <Value name="STA" splitter={true}>(join another network)</Value>
        <Value name="SSID"><Spinner /></Value>
        <Value name="Pass"><Spinner /></Value>
        <Value name="AP" splitter={true}>(own access point)</Value>
        <Value name="SSID"><Spinner /></Value>
        <Value name="Pass"><Spinner /></Value>
        <Value name="Hostname"><Spinner /></Value>
        <Value name="USB mode"><Spinner /></Value>
    {:then json}
        <Value name="Mode">
            <Select
                bind:this={mode_select}
                items={[
                    { text: "STA (join another network)", value: "STA" },
                    { text: "AP (own access point)", value: "AP" },
                    { text: "Disabled (do not use WiFi)", value: "Disabled" },
                ]}
                value={json.wifi_mode}
            />
        </Value>

        <Value name="STA" splitter={true}>(join another network)</Value>

        <Value name="SSID">
            <Input value={json.sta_ssid} bind:this={sta_ssid_input} />
            <ButtonInline value="+" on:click={popup_select_net.show} />
        </Value>

        <Value name="Pass">
            <Input value={json.sta_pass} bind:this={sta_pass_input} />
        </Value>

        <Value name="AP" splitter={true}>(own access point)</Value>

        <Value name="SSID">
            <Input value={json.ap_ssid} bind:this={ap_ssid_input} />
        </Value>

        <Value name="Pass">
            <Input value={json.ap_pass} bind:this={ap_pass_input} />
        </Value>

        <Value name="Hostname">
            <Input value={json.hostname} bind:this={hostname_input} />
        </Value>

        <Value name="USB mode">
            <Select
                bind:this={usb_mode_select}
                items={[
                    { text: "BlackMagicProbe", value: "BM" },
                    { text: "DapLink", value: "DAP" },
                ]}
                value={json.usb_mode}
            />
        </Value>
    {:catch error}
        <error>{error.message}</error>
    {/await}
</Grid>

<div style="margin-top: 10px;">
    <Button value="SAVE" on:click={save_settings} />
    <Button value="REBOOT" on:click={reboot_board} />
</div>

<Popup bind:this={popup_select_net}>
    {#await api.get("/api/v1/wifi/list", {})}
        <div>Nets: <SpinnerBig /></div>
    {:then json}
        <div>Nets:</div>
        {#each json.net_list as net}
            <div>
                <ButtonInline
                    style="normal"
                    value="[{net.ssid} {net.channel}ch {net.rssi}dBm {net.auth}]"
                    on:click={() => {
                        popup_select_net.close();
                        sta_ssid_input.set_value(net.ssid);
                    }}
                />
            </div>
        {/each}
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
