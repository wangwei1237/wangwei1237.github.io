---
name: model-deploy
description: Use this skill when users request to deploy LLMs (Qwen, DeepSeek, etc.) on specified GPU servers and start the model service. This skill can Download models using ModelScope; Start the vLLM inference service.
---

# Model Deploy

在 GPU 服务器上使用 vLLM 部署大语言模型。**目前仅支持在 vLLM 推理引擎上部署大模型**。

## 快速开始
在 ModelScope 平台，模型一般采用  `<MODEL_ORG>/<MODEL_NAME>` 来唯一识别，例如 `Qwen/Qwen3.5-0.8B` 而言，`MODEL_ORG` 为 Qwen，`MODEL_NAME` 为 Qwen3.5-0.8B。

### Qwen 系列模型部署
对于 Qwen 系的模型部署，请使用 `scripts/deploy.sh` 部署脚本，该脚本的使用方式如下：

```bash
用法: [环境变量] deploy.sh <model_name>

示例:
  PORT=8001 \
  GPU_COUNT=4 \
  ./deploy.sh Qwen3.5-0.8B

环境变量:
  ENV_NAME        conda 环境名称 (默认: vllm)
  PORT            服务端口 (默认: 8000)
  GPU_COUNT       GPU 并行数 (默认: 1)
  PROXY           代理地址 (默认: http://{proxyaddress}:{port})
  MODEL_BASE_PATH 模型存储路径 (默认: /home/work/models)
``` 

| 变量 | 说明 | 默认值 |
|-----|------|-------|
| MODEL_ORG | 模型组织 | Qwen |
| MODEL_NAME | 模型名称 | Qwen3.5-0.8B |
| ENV_NAME | conda 环境 | vllm |
| PORT | 模型服务端口 | 8000 |
| GPU_COUNT | GPU 并行数 | 1 |
| PROXY | 代理地址 | http://{proxyaddress}:{port} |
| MODEL_BASE_PATH | 模型本地存储路径 | /home/work/models |

## 部署步骤
- 从用户请求中提取需要部署的：模型名称（MODEL_NAME），模型组织（MODEL_ORG），需要部署的服务器地址（TARGET_HOST），部署账号（TARGET_USER）等所需要的信息。

- 把 `./skills/model-deploy/scripts/deploy.sh` 复制到目标服务器的指定路径下，例如 `$HOME/wangwei17`。
- 在目标服务器上给部署脚本增加可执行权限。
- 在目标服务器上执行部署脚本。必须采用如下的方式来部署：
```bash
ssh ${TARGET_USER}@${TARGET_HOST} "cd $HOME/17 && PORT=8001 && ./deploy.sh Qwen3.5-0.8B"
```
- 部署完成后，在目标服务器上执行 `curl http://127.0.0.1:8001/v1/chat/completions` 测试模型服务是否启动成功。
```bash
curl -X POST http://127.0.0.1:8001/v1/chat/completions \
  -H "Content-Type: application/json" \
  -d '{
      "messages": [{"role": "user", "content": "你好"}],
      "max_tokens": 512
  }'
```

## 约束说明
- 在目标服务器上执行命令必须采用如下的方式：`ssh ${TARGET_USER}@${TARGET_HOST} "${CMD}"`。

## 常见问题
- **端口占用**: 检查 `netstat -tlnp | grep <port>`
- **版本问题**: 运行 `pip install vllm --upgrade`
- **网络问题**: 配置代理 `export https_proxy="http://{proxyaddress}:{port}"`
- **GPU memory 不足**: 运行 `nvidia-smi` 查看显存使用情况，找到满足显存需求的的 GPU 卡编号 GPU_FAN，然后运行 `export CUDA_VISIBLE_DEVICES=$GPU_FAN` 来指定部署使用的 GPU 卡，然后重新执行部署脚本。