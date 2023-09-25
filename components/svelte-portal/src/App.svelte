<script>
  import WebSocket from "./lib/WebSocket.svelte";
  import UartTerminal from "./lib/UartTerminal.svelte";
  import TabWiFi from "./tabs/TabWiFi.svelte";
  import TabSys from "./tabs/TabSys.svelte";
  import TabPs from "./tabs/TabPS.svelte";
  import Reload from "./lib/Reload.svelte";
  import Indicator from "./lib/Indicator.svelte";
  import { onMount } from "svelte";

  let current_tab = "WiFi";
  if (localStorage.getItem("current_tab") != null) {
    current_tab = localStorage.getItem("current_tab");
  }

  function change_tab(tab) {
    current_tab = tab;
    localStorage.setItem("current_tab", current_tab);
  }

  let uart_history_array = [];
  function uart_history_array_get() {
    return uart_history_array;
  }

  function uart_history_array_put(data) {
    uart_history_array.push(data);
  }

  let uart_indicatior = undefined;
  let uart_terminal = undefined;
  let web_socket = undefined;

  function receive_uart(data) {
    uart_indicatior.activate();
    uart_history_array_put(data);
    if (uart_terminal != undefined) {
      uart_terminal.push(data);
    }
  }

  function uart_on_mount() {
    let uart_data = uart_history_array_get();
    for (let i = 0; i < uart_data.length; i++) {
      uart_terminal.push(uart_data[i]);
    }
  }

  function uart_send(data) {
    web_socket.send(data);
  }

  const tabs = ["WiFi", "SYS", "PS", "UART"];

  // ugly hack for terminal height on mobile devices
  const appHeight = () => {
    const doc = document.documentElement;
    doc.style.setProperty("--app-height", `${window.innerHeight}px`);
  };

  onMount(() => {
    appHeight();
    window.addEventListener("resize", appHeight);
    window.addEventListener("orientationchange", function () {
      appHeight();
    });
  });
</script>

<main>
  <tabs>
    {#each tabs as tab}
      <tab
        class:selected={current_tab == tab}
        on:click={() => {
          change_tab(tab);
        }}
        on:keypress={() => {
          change_tab(tab);
        }}
      >
        {tab}
      </tab>
    {/each}
  </tabs>

  <tabs-content class:uart-terminal={current_tab == tabs[3]}>
    {#if current_tab == tabs[0]}
      <tab-content>
        <TabWiFi />
      </tab-content>
    {:else if current_tab == tabs[1]}
      <tab-content>
        <TabSys />
      </tab-content>
    {:else if current_tab == tabs[2]}
      <tab-content>
        <TabPs />
      </tab-content>
    {:else if current_tab == tabs[3]}
      <tab-content class="uart-terminal">
        <UartTerminal
          bind:this={uart_terminal}
          on_mount={uart_on_mount}
          send={uart_send}
        />
      </tab-content>
    {/if}
  </tabs-content>

  <Indicator bind:this={uart_indicatior} />
  <WebSocket bind:this={web_socket} receive={receive_uart} />
  <Reload />
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

  :global(.selectable) {
    -moz-user-select: text;
    -o-user-select: text;
    -khtml-user-select: text;
    -webkit-user-select: text;
    -ms-user-select: text;
    user-select: text;
  }

  :global(error) {
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

  tabs-content.uart-terminal {
    height: calc(var(--app-height) - 105px);
  }

  @media (max-width: 520px) {
    :global(.mobile-hidden) {
      display: none !important;
    }
    main {
      margin: 0;
    }

    tabs-content.uart-terminal {
      height: calc(var(--app-height) - 85px);
    }
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

  tab:last-child {
    margin-right: 0;
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

  tab-content {
    display: block;
  }

  tab-content.uart-terminal {
    height: 100%;
  }
</style>
