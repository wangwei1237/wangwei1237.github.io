#!/bin/bash
######################################################### 
# model_deploy.sh
# vLLM 通用大模型部署脚本模板
# 使用方法: ./model_deploy.sh <model_name> <model_org> [options]
#########################################################

set -e

# ========== 默认配置，通过参数修改 ==========
MODEL_NAME="${1:-Qwen3.5-0.8B}"
MODEL_ORG="${MODEL_ORG:-Qwen}"

# ========== 默认配置，一般不用修改通过环境变量修改 ==========
PORT="${PORT:-8000}"
GPU_COUNT="${GPU_COUNT:-1}"
ENV_NAME="${ENV_NAME:-vllm}"
PROXY="${PROXY:-http://proxyaddress:port}"
MODEL_BASE_PATH="${MODEL_BASE_PATH:-/home/work/models}"
MODEL_SCOPE="/home/work/miniconda3/bin/modelscope"

# ========== 设置代理 ==========
export https_proxy="${PROXY}"
export http_proxy="${PROXY}"

# ========== 颜色输出 ==========
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

log_step() {
    echo -e "${BLUE}[STEP]${NC} $1"
}

# ========== 帮助信息 ==========
show_help() {
    echo "用法: [环境变量] $0 <model_name> <model_org> "
    echo ""
    echo "示例:"
    echo "  PORT=8001 \\"
    echo "  GPU_COUNT=4 \\"
    echo "  $0 Qwen3.5-0.8B Qwen"
    echo ""
    echo "环境变量:"
    echo "  ENV_NAME        conda 环境名称 (默认: vllm)"
    echo "  PORT            服务端口 (默认: 8000)"
    echo "  GPU_COUNT       GPU 并行数 (默认: 1)"
    echo "  PROXY           代理地址 (默认: http://proxyaddress:port)"
    echo "  MODEL_BASE_PATH 模型存储路径 (默认: /home/work/models)"
    exit 0
}

# 定义打印参数说明的函数
print_env_info() {
    # 1. 仅存储变量名的数组（核心：只存变量，不存描述）
    local var_names=(
        "MODEL_NAME"
        "MODEL_ORG"
        "ENV_NAME"
        "PORT"
        "GPU_COUNT"
        "PROXY"
        "MODEL_BASE_PATH"
    )

    # 2. 循环遍历变量名数组，匹配描述并打印
    for var in "${var_names[@]}"; do
        # 关键语法：${!var} → 间接引用变量（通过变量名取变量值）
        local var_value="${!var}"
        
        # 处理未赋值的变量（给默认值，可选）
        if [[ -z "$var_value" ]]; then
            case "$var" in
                PORT) var_value="8000" ;;          # PORT 默认8000
                *) var_value="(未赋值/默认值)" ;;  # 其他变量默认提示
            esac
        fi

        # 格式化打印：变量名左对齐占16字符，值紧跟
        printf "  %-16s %s\n" "$var:" "$var_value"
    done
}

# ========== 初始化 conda ==========
init_conda() {
    local CONDA_INIT="$HOME/miniconda3/etc/profile.d/conda.sh"
    if [ -f "$CONDA_INIT" ]; then
        . "$CONDA_INIT"
        log_info "conda 初始化成功..."
    else
        log_error "conda 初始化脚本不存在: $CONDA_INIT"
        exit 1
    fi
}

# ========== 下载模型 ==========
download_model() {
    local MODEL_PATH="${MODEL_BASE_PATH}/${MODEL_NAME}"

    log_step "下载模型..."

    if [ -f "${MODEL_PATH}/config.json" ]; then
        log_info "模型已存在: ${MODEL_PATH}，避免模型文件未下载完毕的问题..."
        $MODEL_SCOPE download \
            --model ${MODEL_ORG}/${MODEL_NAME} \
            --local_dir ${MODEL_PATH}
        return 0
    fi

    log_info "创建模型目录: ${MODEL_PATH}"
    mkdir -p ${MODEL_PATH}

    log_info "从 ModelScope 下载模型: ${MODEL_ORG}/${MODEL_NAME}"
    log_info "这可能需要较长时间，请耐心等待..."

    $MODEL_SCOPE download \
        --model ${MODEL_ORG}/${MODEL_NAME} \
        --local_dir ${MODEL_PATH}

    if [ ! -f "${MODEL_PATH}/config.json" ]; then
        log_error "模型下载失败"
        exit 1
    fi

    log_info "✅ 模型下载完成"
}

# ========== 检查环境 ==========
check_environment() {
    log_step "检查部署环境..."

    # 检查 conda 环境
    if ! conda env list | grep -q "^${ENV_NAME} "; then
        log_warn "conda 环境 '${ENV_NAME}' 不存在"
        exit 1
    else
        log_info "使用已有环境: ${ENV_NAME}"
        conda activate ${ENV_NAME}
    fi

    # 检查端口
    if netstat -tlnp 2>/dev/null | grep -q ":${PORT} "; then
        log_warn "端口 ${PORT} 已被占用"
        local NEW_PORT=$((PORT + 1))
        log_info "自动切换到端口: ${NEW_PORT}"
        PORT=${NEW_PORT}
    fi
}

# ========== 启动服务 ==========
start_service() {
    local MODEL_PATH="${MODEL_BASE_PATH}/${MODEL_NAME}"

    log_step "启动 vLLM 服务..."

    echo ""
    log_info "============================================"
    log_info "模型: ${MODEL_NAME}"
    log_info "路径: ${MODEL_PATH}"
    log_info "端口: ${PORT}"
    log_info "GPU数: ${GPU_COUNT}"
    log_info "访问: http://localhost:${PORT}"
    log_info "============================================"
    log_info "按 Ctrl+C 停止服务"
    echo ""

    vllm serve ${MODEL_PATH} \
        --tensor-parallel-size ${GPU_COUNT} \
        --gpu-memory-utilization 0.9 \
        --host 0.0.0.0 \
        --port ${PORT}
}

# ========== 本地部署函数 ==========
deploy_model() {
    log_info "🐵 部署模型: ${MODEL_NAME}"
    echo ""

    init_conda
    download_model
    check_environment
    start_service
}

# ========== 主流程 ==========
main() {
    MODEL_NAME=$1

    # 条件判断逻辑
    if [ $# -eq 0 ]; then
        show_help
    elif [ $# -eq 1 ]; then
        MODEL_NAME=$1
    elif [ $# -eq 2 ]; then
        MODEL_ORG=$2
    else
        echo "仅支持 0/1/2 个参数....."
    fi

    print_env_info

    deploy_model
}

# ========== 启动 ==========
main "$@"