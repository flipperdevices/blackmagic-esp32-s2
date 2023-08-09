<script>
</script>

<spinner />

<style>
  @keyframes spinner-animation {
    0% {
      content: "|";
    }
    25% {
      content: "/";
    }
    50% {
      content: "-";
    }
    75% {
      content: "\\";
    }
    100% {
      content: "|";
    }
  }

  spinner::after {
    display: inline-block;
    animation: spinner-animation 0.6s linear infinite alternate;
    content: "|";
  }
</style>
