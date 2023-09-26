<script>
    import { api } from "./Api.svelte";
    import Button from "./Button.svelte";
</script>

{#if api.dev_mode}
    <div style="position: fixed; bottom: 0; right: 0;">
        <Button
            value="RELOAD"
            on:click={() => {
                location.reload();
            }}
        />
    </div>
{/if}
