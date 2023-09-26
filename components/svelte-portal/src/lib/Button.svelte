<script>
  export let value = "Value";
  //export let click;

  let left = "";
  let right = "";
  let timer = null;

  function reset_brace() {
    left = "[";
    right = "]";
  }

  function set_brace() {
    left = ">";
    right = "<";
  }

  function timer_click() {
    if (left == "[") {
      set_brace();
    } else {
      reset_brace();
    }
  }

  function mouseenter() {
    if (timer == null) {
      timer = setInterval(timer_click, 400);
    }
    set_brace();
  }

  function mouseleave() {
    if (timer != null) {
      clearInterval(timer);
      timer = null;
    }
    reset_brace();
  }

  reset_brace();
</script>

<input
  type="button"
  value={left + value + right}
  class="button-css"
  on:mouseenter={mouseenter}
  on:mouseleave={mouseleave}
  on:click
/>

<style>
  .button-css {
    background-color: black;
    color: white;
    font-size: 28px;
    font-family: "DOS", monospace;
    line-height: 1;
    -webkit-text-size-adjust: 100%;
    -webkit-tap-highlight-color: rgba(0, 0, 0, 0);
    border: 0;
    padding: 5px 10px;
    display: inline-block;
    max-width: 100%;
  }

  .button-css:hover {
    background: rgb(255, 255, 255);
    color: #000000;
  }
</style>
