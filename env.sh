#!/bin/bash
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
alias build='${SCRIPT_DIR}/build.sh build'
alias rebuild='${SCRIPT_DIR}/build.sh rebuild'
alias clean='${SCRIPT_DIR}/build.sh clean'
alias run='${SCRIPT_DIR}/tmp/VideoRecorder'
