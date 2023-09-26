<script>
    import { api } from "../lib/Api.svelte";
    import Grid from "../lib/Grid.svelte";
    import Spinner from "../lib/Spinner.svelte";
    import Value from "../lib/Value.svelte";

    function print_mac(mac_array) {
        let str = "";
        for (let index = 0; index < mac_array.length; index++) {
            str += mac_array[index].toString(16).padStart(2, "0");
            if (index < mac_array.length - 1) {
                str += ":";
            }
        }
        return str;
    }

    function print_ip(ip_addr) {
        var byteArray = [0, 0, 0, 0];

        for (var index = 0; index < byteArray.length; index++) {
            var byte = ip_addr & 0xff;
            byteArray[index] = byte;
            ip_addr = ip_addr >> 8;
        }

        return byteArray.join(".");
    }
</script>

<!-- 
"firmware_commit":	"03b806d",
"firmware_branch":	"zlo/2630-logs-over-wifi",
"firmware_branch_num":	"157",
"firmware_version":	"0.1.1",
"firmware_build_date":	"22-09-2023", 
-->
<Grid>
    {#await api.get("/api/v1/system/info")}
        <Value name="IP"><Spinner /></Value>
        <Value name="Mac"><Spinner /></Value>
        <Value name="IDF ver"><Spinner /></Value>
        <Value name="Model"><Spinner /></Value>

        <Value name="Heap" splitter={true}>info</Value>
        <Value name="Min free"><Spinner /></Value>
        <Value name="Free"><Spinner /></Value>
        <Value name="Alloc"><Spinner /></Value>
        <Value name="Max block"><Spinner /></Value>

        <Value name="PSRAM" splitter={true}>info</Value>
        <Value name="Min free"><Spinner /></Value>
        <Value name="Free"><Spinner /></Value>
        <Value name="Alloc"><Spinner /></Value>
        <Value name="Max block"><Spinner /></Value>
    {:then json}
        <Value name="IP" selectable="true">{print_ip(json.ip)}</Value>
        <Value name="Mac">{print_mac(json.mac)}</Value>
        <Value name="IDF ver">{json.idf_version}</Value>
        <Value name="FW commit">
            {json.firmware_branch}#{json.firmware_commit}
        </Value>
        <Value name="FW ver">
            {json.firmware_version}/{json.firmware_branch_num}/{json.firmware_build_date}
        </Value>
        <Value name="Model">
            {json.model}.{json.revision}
            {json.cores}-core
        </Value>

        <Value name="Heap" splitter={true}>info</Value>
        <Value name="Min free">{json.heap.minimum_free_bytes}</Value>
        <Value name="Free">{json.heap.total_free_bytes}</Value>
        <Value name="Alloc">{json.heap.total_allocated_bytes}</Value>
        <Value name="Max block">{json.heap.largest_free_block}</Value>

        <Value name="PSRAM" splitter={true}>info</Value>
        <Value name="Min free">{json.psram_heap.minimum_free_bytes}</Value>
        <Value name="Free">{json.psram_heap.total_free_bytes}</Value>
        <Value name="Alloc">{json.psram_heap.total_allocated_bytes}</Value>
        <Value name="Max block">{json.psram_heap.largest_free_block}</Value>
    {:catch error}
        <error>{error.message}</error>
    {/await}
</Grid>
