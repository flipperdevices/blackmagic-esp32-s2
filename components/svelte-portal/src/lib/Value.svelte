<script>
    export let name = "Name";
    export let splitter = false;
    export let selectable = false;
</script>

{#if !splitter}
    <div class="value-name">{name}:</div>
    <div class="value {selectable ? 'selectable' : ''}"><slot /></div>
{:else}
    <div class="value-name splitter">{name}</div>
    <div class="value mobile-hidden">&nbsp;<slot /></div>
{/if}

<style>
    .value {
        display: inline-flex;
    }

    .value-name {
        text-align: right;
    }

    @media (max-width: 520px) {
        .value-name {
            text-align: left;
        }

        .splitter {
            background-color: #000;
            width: 100%;
            color: #ffa21d;
            text-align: center;
        }
    }
</style>
