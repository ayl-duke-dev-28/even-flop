#!/usr/bin/env bash
# Serves the even-flop web UI on localhost.
#
# There is no build step: the WebAssembly engine is committed under web/, so
# this only needs a static file server. Nothing is fetched from the network at
# runtime, so the page also works offline.
#
#   ./serve.sh          # http://localhost:8000
#   ./serve.sh 9000     # pick a port
#   PORT=9000 ./serve.sh
set -euo pipefail

port="${1:-${PORT:-8000}}"
here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
root="$here/web"

if [[ ! "$port" =~ ^[0-9]+$ ]] || (( port < 1 || port > 65535 )); then
  echo "serve.sh: '$port' is not a valid port number." >&2
  exit 2
fi

if [ ! -f "$root/index.html" ]; then
  echo "serve.sh: web/index.html is missing. Run this from a full checkout." >&2
  exit 1
fi

for required in even-flop-engine.js even-flop-wasm.js; do
  if [ ! -f "$root/$required" ]; then
    echo "serve.sh: web/$required is missing." >&2
    echo "  It is committed to the repo; if you deleted it, rebuild with web/build.sh" >&2
    echo "  (that one needs emscripten), or restore it with: git checkout web/" >&2
    exit 1
  fi
done

# Check the port first so the failure is a clear message rather than a stack
# trace from whichever server we picked.
if (exec 3<>"/dev/tcp/127.0.0.1/$port") 2>/dev/null; then
  exec 3<&- 3>&-
  echo "serve.sh: port $port is already in use. Try another: ./serve.sh $((port + 1))" >&2
  exit 1
fi

echo "even-flop  ->  http://localhost:$port"
echo "Press Ctrl+C to stop."
echo

if command -v python3 >/dev/null 2>&1; then
  exec python3 -m http.server "$port" --directory "$root" --bind 127.0.0.1
elif command -v npx >/dev/null 2>&1; then
  exec npx --yes serve --listen "$port" "$root"
elif command -v ruby >/dev/null 2>&1; then
  exec ruby -run -e httpd "$root" -p "$port" -b 127.0.0.1
else
  echo "serve.sh: no python3, npx or ruby found to serve with." >&2
  echo "  You can skip the server entirely and open this file in a browser:" >&2
  echo "  $root/index.html" >&2
  exit 1
fi
