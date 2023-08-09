<div class="grid">
    <slot />
</div>

<style>
    .grid {
        display: inline-grid;
        grid-template-columns: auto auto;
    }

    :global(.grid > div) {
        margin-top: 10px;
    }

    @media (max-width: 520px) {
        .grid {
            grid-template-columns: auto;
            width: 100%;
        }
    }
</style>
