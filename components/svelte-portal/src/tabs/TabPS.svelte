<script>
    import { api } from "../lib/Api.svelte";
    import Spinner from "../lib/Spinner.svelte";
</script>

{#await api.get("/api/v1/system/tasks")}
    <task-list>
        <span><Spinner /></span>
        <span><Spinner /></span>
        <span><Spinner /></span>
        <span><Spinner /></span>
        <span><Spinner /></span>
    </task-list>
{:then json}
    <task-list>
        <span class="mobile-hidden">Name</span>
        <span class="mobile-hidden">State</span>
        <span class="mobile-hidden">Handle</span>
        <span class="mobile-hidden">Stack base</span>
        <span class="mobile-hidden">WMRK</span>
        {#each json.list.sort(function (a, b) {
            return a.number - b.number;
        }) as task}
            <span>{task.name}</span>
            <span>{task.state}</span>
            <span>{task.handle.toString(16).toUpperCase()}</span>
            <span>{task.stack_base.toString(16).toUpperCase()}</span>
            <span>{task.watermark}</span>
        {/each}
    </task-list>
{:catch error}
    <error>{error.message}</error>
{/await}

<style>
    task-list {
        display: inline-grid;
        grid-template-columns: auto auto auto auto auto;
        width: 100%;
    }

    @media (max-width: 768px) {
        task-list {
            grid-template-columns: auto auto auto auto;
        }

        task-list > span:nth-child(5n + 3) {
            display: none;
        }
    }

    @media (max-width: 600px) {
        task-list {
            grid-template-columns: auto auto auto;
        }

        task-list > span:nth-child(5n + 4) {
            display: none;
        }
    }

    @media (max-width: 520px) {
        task-list {
            grid-template-columns: auto;
            text-align: center;
        }

        task-list > span:nth-child(5n + 1) {
            padding-top: 10px;
        }

        task-list > span:nth-child(5n + 5) {
            border-bottom: 4px dashed #000;
        }
    }
</style>
