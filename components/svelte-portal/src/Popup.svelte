<script>
  let closed = true;

  export function close() {
    closed = true;
  }

  export function show() {
    closed = false;
  }
</script>

{#if !closed}
  <popup-wrapper>
    <popup-body>
      <popup-content>
        <popup-close on:click={close}>X</popup-close>
        <popup-border>
          <slot />
        </popup-border>
      </popup-content>
    </popup-body>
  </popup-wrapper>
{/if}

<style>
  popup-wrapper {
    background-color: rgba(0, 0, 0, 0.863);
    width: 100%;
    height: 100%;
    display: table;
    table-layout: fixed;
    z-index: 999;
    overflow: auto;
    position: fixed;
    top: 0;
    left: 0;
    right: 0;
    bottom: 0;
  }

  popup-body {
    margin: auto;
    display: table-cell;
    text-align: center;
    vertical-align: middle;
    width: 100%;
  }

  popup-content {
    background-color: #ffa21c;
    display: inline-block;
    outline: none;
    position: relative;
    text-align: initial;
    max-width: 100vw;
  }

  popup-border {
    display: block;
    border: 4px dashed #000;
    margin: 10px;
    padding: 10px;
  }

  popup-close {
    background-color: #000;
    display: inline-block;
    color: #ffa21c;
    position: absolute;
    width: 24px;
    right: 0px;
    top: 0px;
    text-align: center;
  }
  popup-close:hover {
    background-color: #fff;
    color: #000;
  }
</style>
