<script>
  export let value = "Value";
  export let style = "black";
</script>

<input type="button" {value} class="button {style}" on:click />

<style>
  .button {
    box-sizing: border-box;
    display: inline-block;
    font-size: 28px;
    font-family: "DOS", monospace;
    line-height: 1;
    border: 0;
    padding: 0 5px 0 5px;
    box-shadow: none;
    border-radius: 0;
    display: inline-block;
    max-width: 100%;
  }

  .black {
    color: white;
    background-color: black;
    border-bottom: 4px solid #000;
  }

  .black:hover {
    background: #fff;
    color: #000;
  }

  .normal {
    color: #000;
    background-color: #ffa21c;
    border-bottom: 4px solid #ffa21c;
  }

  .normal:hover {
    background: #000;
    color: #fff;
  }
</style>
