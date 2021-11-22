<script>
  import Button from "./Button.svelte";
  import Popup from "./Popup.svelte";
  import Input from "./Input.svelte";
  import Spinner from "./Spinner.svelte";
  import SpinnerBig from "./SpinnerBig.svelte";
  import Select from "./Select.svelte";
  import { onMount } from "svelte";
  import ButtonInline from "./ButtonInline.svelte";

  // let server = "";
  let server = "http://192.168.31.235";

  async function api_post(api, data) {
    const res = await fetch(api, {
      method: "POST",
      body: JSON.stringify(data),
    });

    const json = await res.json();
    return json;
  }

  async function api_get(api) {
    const res = await fetch(api, {
      method: "GET",
    });

    const json = await res.json();
    return json;
  }

  let popup_select_net;
  let popup_message;
  let popup_message_text;

  let mode_select;
  let ssid_input;
  let password_input;

  onMount(() => {
    //popup_select_net.show();
  });

  async function save_settings() {
    popup_message_text = "";
    popup_message.show();

    await api_post(server + "/api/v1/wifi/set_credenitals", {
      mode: mode_select.get_value(),
      ssid: ssid_input.get_value(),
      pass: password_input.get_value(),
    }).then(() => {
      popup_message_text = "Saved!";
    });
  }
</script>

<main>
  <tabs><tab class="selected">WiFi</tab><tab>SYS</tab><tab>GDB</tab></tabs>
  <tabs-content>
    <tab-content>
      <div class="grid">
        {#await api_get(server + "/api/v1/wifi/get_credenitals")}
          <div class="value-name">Mode:</div>
          <div><Spinner /></div>
          <div class="value-name">SSID:</div>
          <div><Spinner /></div>
          <div class="value-name">Pass:</div>
          <div><Spinner /></div>
        {:then json}
          <div class="value-name">Mode:</div>
          <div>
            <Select
              bind:this={mode_select}
              items={[
                { text: "STA", value: "STA" },
                { text: "AP", value: "AP" },
              ]}
              value={json.mode}
            />
          </div>

          <div class="value-name">SSID:</div>
          <div>
            <Input value={json.ssid} bind:this={ssid_input} /><ButtonInline
              value="+"
              on:click={popup_select_net.show}
            />
          </div>

          <div class="value-name">Pass:</div>
          <div>
            <Input value={json.pass} bind:this={password_input} />
          </div>
        {:catch error}
          <error>{error.message}</error>
        {/await}
      </div>
      <div style="margin-top: 10px;">
        <Button value="SAVE" on:click={save_settings} />
      </div>
    </tab-content>
  </tabs-content>

  <Popup bind:this={popup_select_net}>
    {#await api_get(server + "/api/v1/wifi/list", {})}
      <div>Nets: <SpinnerBig /></div>
    {:then json}
      <div>Nets:</div>
      {#each json.net_list as net}
        <div>
          <ButtonInline
            style="normal"
            value="[{net.ssid} {net.channel}ch {net.rssi}db {net.auth}]"
            on:click={() => {
              popup_select_net.close();
              ssid_input.set_value(net.ssid);
            }}
          />
        </div>
      {/each}
    {:catch error}
      <error>{error.message}</error>
    {/await}
  </Popup>

  <Popup bind:this={popup_message}>
    {#if popup_message_text != ""}
      {popup_message_text}
    {:else}
      <Spinner />
    {/if}
  </Popup>
</main>

<style>
  main {
    border: 4px dashed #000;
    margin: 10px auto;
    padding: 10px;
    max-width: 500px;
  }

  * {
    -moz-user-select: none;
    -o-user-select: none;
    -khtml-user-select: none;
    -webkit-user-select: none;
    -ms-user-select: none;
    user-select: none;
  }

  tabs {
    border-bottom: 4px dashed #000;
    width: 100%;
    display: block;
  }

  tab {
    margin-right: 10px;
    padding: 5px 10px;
    margin-bottom: 5px;
    display: inline-block;
  }

  tab:hover,
  tab.selected:hover {
    background: rgb(255, 255, 255);
    color: #000000;
  }

  tab.selected {
    background-color: black;
    color: white;
  }

  tabs-content {
    display: block;
    margin-top: 10px;
  }

  error {
    padding: 5px 10px;
    background-color: rgb(255, 0, 0);
    color: black;
  }

  @font-face {
    font-family: "DOS";
    src: url("../assets/ega8.otf") format("opentype");
    font-weight: normal;
    font-style: normal;
    -webkit-font-kerning: none;
    font-kerning: none;
    font-synthesis: none;
    -webkit-font-variant-ligatures: none;
    font-variant-ligatures: none;
    font-variant-numeric: tabular-nums;
  }

  :global(body) {
    padding: 0;
    margin: 0;
    background-color: #ffa21c;
    color: #000;
    font-size: 28px;
    font-family: "DOS", monospace;
    line-height: 1;
    -webkit-text-size-adjust: 100%;
    -webkit-tap-highlight-color: rgba(0, 0, 0, 0);
  }

  .grid {
    display: inline-grid;
    grid-template-columns: auto auto;
  }

  .grid > div {
    margin-top: 10px;
  }

  .value-name {
    text-align: right;
  }
</style>
