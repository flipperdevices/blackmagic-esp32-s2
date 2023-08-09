<script>
  import WebSocket from "./lib/WebSocket.svelte";
  import UartTerminal from "./lib/UartTerminal.svelte";
  import TabWiFi from "./tabs/TabWiFi.svelte";
  import TabSys from "./tabs/TabSys.svelte";
  import TabPs from "./tabs/TabPS.svelte";
  import Reload from "./lib/Reload.svelte";

  let current_tab = "WiFi";
  if (localStorage.getItem("current_tab") != null) {
    current_tab = localStorage.getItem("current_tab");
  }

  function change_tab(tab) {
    current_tab = tab;
    localStorage.setItem("current_tab", current_tab);
  }

  let uart_terminal = undefined;
  function receive_uart(data) {
    if (uart_terminal != undefined) {
      uart_terminal.push(data);
    }
  }

  const tabs = ["WiFi", "SYS", "PS", "UART"];
</script>

<main>
  <tabs>
    {#each tabs as tab}
      <tab
        class:selected={current_tab == tab}
        on:click={() => {
          change_tab(tab);
        }}
      >
        {tab}
      </tab>
    {/each}
  </tabs>

  <tabs-content>
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
      <tab-content>
        <WebSocket receive={receive_uart} />
        <UartTerminal bind:this={uart_terminal} />
      </tab-content>
    {/if}
  </tabs-content>

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

  @media (max-width: 520px) {
    :global(.mobile-hidden) {
      display: none !important;
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
</style>
