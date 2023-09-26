<script>
  export let items = [];
  export let value = "";

  export function get_value() {
    return value;
  }

  function on_change() {
    value = this.value;
  }
</script>

<select bind:value on:change={on_change}>
  {#each items as item}
    <option value={item.value}>
      {item.text}
    </option>
  {/each}
</select>

<style>
  select {
    display: inline-block;
    color: #000;
    font-size: 28px;
    font-family: "DOS", monospace;
    line-height: 1;
    box-sizing: border-box;
    margin: 0;
    border: 0;
    border-bottom: 4px solid #000;
    padding: 0 5px 0 5px;
    box-shadow: none;
    border-radius: 0;
    -moz-appearance: none;
    -webkit-appearance: none;
    appearance: none;
    background-color: #ffa21c;
  }

  select::-ms-expand {
    display: none;
  }
  select:hover {
    background: rgb(255, 255, 255);
    color: #000000;
  }
  select:focus {
    box-shadow: none;
    outline: none;
    background: rgb(255, 255, 255);
    color: #000000;
  }
  select option {
    font-weight: normal;
  }

  @media (max-width: 520px) {
    select {
      width: 100%;
    }
  }
</style>
