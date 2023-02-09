<script>
  import Button from "./Button.svelte";
  import Popup from "./Popup.svelte";
  import Input from "./Input.svelte";
  import Spinner from "./Spinner.svelte";
  import SpinnerBig from "./SpinnerBig.svelte";
  import Select from "./Select.svelte";
  import ButtonInline from "./ButtonInline.svelte";

  let server = "";
  if (development_mode) {
    server = "http://172.30.1.223";
  }

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
  let usb_mode_select;
  let ap_ssid_input;
  let ap_pass_input;
  let sta_ssid_input;
  let sta_pass_input;
  let hostname_input;

  let current_tab = "WiFi";
  if (localStorage.getItem("current_tab") != null) {
    current_tab = localStorage.getItem("current_tab");
  }

  async function save_settings() {
    popup_message_text = "";
    popup_message.show();

    await api_post(server + "/api/v1/wifi/set_credentials", {
      wifi_mode: mode_select.get_value(),
      usb_mode: usb_mode_select.get_value(),
      ap_ssid: ap_ssid_input.get_value(),
      ap_pass: ap_pass_input.get_value(),
      sta_ssid: sta_ssid_input.get_value(),
      sta_pass: sta_pass_input.get_value(),
      hostname: hostname_input.get_value(),
    }).then((json) => {
      if (json.error) {
        popup_message_text = json.error;
      } else {
        popup_message_text = "Saved!";
      }
    });
  }

  async function reboot_board() {
    api_post(server + "/api/v1/system/reboot", {});
    popup_message_text = "Rebooted";
    popup_message.show();
  }

  function change_tab(tab) {
    current_tab = tab;
    localStorage.setItem("current_tab", current_tab);
  }

  function print_mac(mac_array) {
    let str = "";
    for (let index = 0; index < mac_array.length; index++) {
      str += mac_array[index].toString(16).padStart(2, "0");
      if (index < mac_array.length - 1) {
        str += ":";
      }
    }
    return str;
  }

  function print_ip(ip_addr) {
    var byteArray = [0, 0, 0, 0];

    for (var index = 0; index < byteArray.length; index++) {
      var byte = ip_addr & 0xff;
      byteArray[index] = byte;
      ip_addr = ip_addr >> 8;
    }

    return byteArray.join(".");
  }
</script>

<main>
  <tabs>
    <tab
      class:selected={current_tab == "WiFi"}
      on:click={() => {
        change_tab("WiFi");
      }}
    >
      WiFi
    </tab>

    <tab
      class:selected={current_tab == "SYS"}
      on:click={() => {
        change_tab("SYS");
      }}
    >
      SYS
    </tab>

    <tab
      class:selected={current_tab == "PS"}
      on:click={() => {
        change_tab("PS");
      }}
    >
      PS
    </tab>
  </tabs>

  <tabs-content>
    {#if current_tab == "WiFi"}
      <tab-content>
        <div class="grid">
          {#await api_get(server + "/api/v1/wifi/get_credentials")}
            <div class="value-name">Mode:</div>
            <div class="value"><Spinner /></div>

            <div class="value-name splitter">STA</div>
            <div class="value mobile-hidden">(join another network)</div>

            <div class="value-name">SSID:</div>
            <div class="value"><Spinner /></div>

            <div class="value-name">Pass:</div>
            <div class="value"><Spinner /></div>

            <div class="value-name splitter">AP</div>
            <div class="value mobile-hidden">(own access point)</div>

            <div class="value-name">SSID:</div>
            <div class="value"><Spinner /></div>

            <div class="value-name">Pass:</div>
            <div>class="value"<Spinner /></div>

            <div class="value-name">Hostname:</div>
            <div class="value"><Spinner /></div>

            <div class="value-name">USB mode:</div>
            <div class="value"><Spinner /></div>
          {:then json}
            <div class="value-name">Mode:</div>
            <div class="value">
              <Select
                bind:this={mode_select}
                items={[
                  { text: "STA (join another network)", value: "STA" },
                  { text: "AP (own access point)", value: "AP" },
                  { text: "Disabled (do not use WiFi)", value: "Disabled" },
                ]}
                value={json.wifi_mode}
              />
            </div>

            <div class="value-name splitter">STA</div>
            <div class="value mobile-hidden">(join another network)</div>

            <div class="value-name">SSID:</div>
            <div class="value">
              <Input
                value={json.sta_ssid}
                bind:this={sta_ssid_input}
              /><ButtonInline value="+" on:click={popup_select_net.show} />
            </div>

            <div class="value-name">Pass:</div>
            <div class="value">
              <Input value={json.sta_pass} bind:this={sta_pass_input} />
            </div>

            <div class="value-name splitter">AP</div>
            <div class="value mobile-hidden">(own access point)</div>

            <div class="value-name">SSID:</div>
            <div class="value">
              <Input value={json.ap_ssid} bind:this={ap_ssid_input} />
            </div>

            <div class="value-name">Pass:</div>
            <div class="value">
              <Input value={json.ap_pass} bind:this={ap_pass_input} />
            </div>

            <div class="value-name">Hostname:</div>
            <div class="value">
              <Input value={json.hostname} bind:this={hostname_input} />
            </div>

            <div class="value-name">USB mode:</div>
            <div class="value">
              <Select
                bind:this={usb_mode_select}
                items={[
                  { text: "BlackMagicProbe", value: "BM" },
                  { text: "DapLink", value: "DAP" },
                ]}
                value={json.usb_mode}
              />
            </div>
          {:catch error}
            <error>{error.message}</error>
          {/await}
        </div>
        <div style="margin-top: 10px;">
          <Button value="SAVE" on:click={save_settings} />
          <Button value="REBOOT" on:click={reboot_board} />
        </div>
      </tab-content>
    {/if}

    {#if current_tab == "SYS"}
      <tab-content>
        <div class="grid">
          {#await api_get(server + "/api/v1/system/info")}
            <div class="value-name">IP:</div>
            <div class="value"><Spinner /></div>
          {:then json}
            <div class="value-name">IP:</div>
            <div class="value">{print_ip(json.ip)}</div>
            <div class="value-name">Mac:</div>
            <div class="value">{print_mac(json.mac)}</div>
            <div class="value-name">IDF ver:</div>
            <div class="value">{json.idf_version}</div>
            <div class="value-name">Model:</div>
            <div class="value">
              {json.model}.{json.revision}
              {json.cores}-core
            </div>
            <div class="value-name">Min free:</div>
            <div class="value">{json.heap.minimum_free_bytes}</div>
            <div class="value-name">Free:</div>
            <div class="value">{json.heap.total_free_bytes}</div>
            <div class="value-name">Alloc:</div>
            <div class="value">{json.heap.total_allocated_bytes}</div>
            <div class="value-name">Max block:</div>
            <div class="value">{json.heap.largest_free_block}</div>
          {:catch error}
            <error>{error.message}</error>
          {/await}
        </div>
      </tab-content>
    {/if}

    {#if current_tab == "PS"}
      <tab-content>
        {#await api_get(server + "/api/v1/system/tasks")}
          <span>Name</span>
          <span><Spinner /></span>
        {:then json}
          <task-list>
            <span class="mobile-hidden">Name</span>
            <span class="mobile-hidden">State</span>
            <span class="mobile-hidden">Handle</span>
            <span class="mobile-hidden">Stack base</span>
            <span class="mobile-hidden">WMRK</span>
            {#each json.list.sort(function (a, b) {
              return a.number - b.number;
            }) as task}
              <span>{task.name}</span>
              <span>{task.state}</span>
              <span>0x{task.handle.toString(16).toUpperCase()}</span>
              <span>0x{task.stack_base.toString(16).toUpperCase()}</span>
              <span>{task.watermark}</span>
            {/each}
          </task-list>
        {:catch error}
          <error>{error.message}</error>
        {/await}
      </tab-content>
    {/if}
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
    max-width: 800px;
    overflow: hidden;
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

  task-list {
    display: inline-grid;
    grid-template-columns: auto auto auto auto auto;
    width: 100%;
  }

  @media (max-width: 768px) {
    task-list {
      grid-template-columns: auto auto auto auto;
    }

    task-list > span:nth-child(5n + 3) {
      display: none;
    }
  }

  @media (max-width: 600px) {
    task-list {
      grid-template-columns: auto auto auto;
    }

    task-list > span:nth-child(5n + 4) {
      display: none;
    }
  }

  @media (max-width: 520px) {
    .grid {
      grid-template-columns: auto;
      width: 100%;
    }

    .mobile-hidden {
      display: none;
    }

    .value-name {
      text-align: left;
    }

    .splitter {
      background-color: #000;
      width: 100%;
      color: #ffa21d;
      text-align: center;
    }

    task-list {
      grid-template-columns: auto;
      text-align: center;
    }

    task-list > span:nth-child(5n + 1) {
      padding-top: 10px;
    }

    task-list > span:nth-child(5n + 5) {
      border-bottom: 4px dashed #000;
    }
  }
</style>
