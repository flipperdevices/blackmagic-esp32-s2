<script>
    let active = false;
    let timer = undefined;

    export function activate() {
        active = true;

        if (timer != undefined) {
            clearTimeout(timer);
        }

        timer = setTimeout(() => {
            active = false;
        }, 100);
    }
</script>

<div class="indicatior" class:active>U</div>

<style>
    .indicatior {
        position: fixed;
        top: 0;
        right: 0;
        background-color: green;
        color: white;
        padding: 4px;
        visibility: hidden;
        pointer-events: none;
    }

    .indicatior.active {
        visibility: visible;
    }
</style>
