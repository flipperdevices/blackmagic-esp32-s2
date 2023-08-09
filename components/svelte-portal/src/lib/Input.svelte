<script>
  export let value = "";

  export function set_value(new_value) {
    value = new_value;
  }

  export function get_value() {
    return value;
  }

  function text_input() {
    this.size = this.value.length > 3 ? this.value.length : 3;
    value = this.value;
  }
</script>

<input
  autocorrect="off"
  autocapitalize="none"
  autocomplete="off"
  type="text"
  {value}
  size={value.length > 3 ? value.length : 3}
  on:input={text_input}
/>

<style>
  input {
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
    height: 32px;
  }

  input:focus-visible,
  input:hover {
    outline: 0;
    background-color: white;
  }

  @media (max-width: 520px) {
    input {
      max-width: 100%;
    }
  }
</style>
