#!/usr/bin/env python3
"""Answer a file of chat prompts with one model loaded in this process (vLLM's offline LLM class).

No server, no HTTP, no ports for requests: prompts are read from a JSONL file and answers written to
another. The vLLM engine runs inside this process (VLLM_ENABLE_V1_MULTIPROCESSING=0). Called by
synthetic/generator/generate.py (GEN_LLM=offline) and scripts/agent_synthetic.py, one model per call; the GPU is free again
when the process exits.

    synthetic/generator/llm_batch.py --model google/gemma-4-31B-it-qat-w4a16-ct --requests R.jsonl --answers A.jsonl
    R.jsonl: {"id": ..., "messages": [{"role": ..., "content": ...}, ...], "seed": ...}
             or {"id": ..., "system": ..., "user": ..., "seed": ...}
    A.jsonl: {"id": ..., "text": ..., "usage": {"prompt_tokens": ..., "completion_tokens": ...},
              "finish_reason": ...}

Requests whose prompt does not fit the context get an empty text and finish_reason "too_long".
"""
from __future__ import annotations

import argparse
import json
import os
import sys
import time

os.environ.setdefault("VLLM_ENABLE_V1_MULTIPROCESSING", "0")  # engine in this process, no IPC sockets


class InProcessLLM:
    """One model in this process; generate() answers a batch of conversations. Used by main() below and
    by scripts/agent_synthetic.py (many agent conversations advanced together, one step per batch)."""

    def __init__(self, model: str, max_model_len: int = 32768, gpu_memory_utilization: float = 0.90):
        from vllm import LLM  # noqa: E402  after the environment is set
        t0 = time.time()
        kw = dict(model=model, max_model_len=max_model_len, gpu_memory_utilization=gpu_memory_utilization, seed=0)
        try:
            self.llm = LLM(**kw, safetensors_load_strategy="prefetch")  # BeeGFS: read the weights in one pass
        except TypeError:
            self.llm = LLM(**kw)
        self.tok = self.llm.get_tokenizer()
        self.max_model_len = max_model_len
        print(f"llm_batch: {model} loaded in {time.time() - t0:.0f} s", flush=True)

    def prompt_tokens(self, messages: list[dict]) -> int:
        text = self.tok.apply_chat_template(messages, add_generation_prompt=True, tokenize=False)
        return len(self.tok.encode(text, add_special_tokens=False))  # the template holds the special tokens

    def generate(self, convs: list[list[dict]], seeds: list[int], temperature: float, top_p: float, top_k: int,
                 max_tokens: int) -> list[dict]:
        """[{"text", "usage", "finish_reason"}] per conversation; finish_reason "too_long" (empty text) when
        the prompt does not fit the context."""
        from vllm import SamplingParams  # noqa: E402
        out: list[dict | None] = [None] * len(convs)
        fits = []
        for i, m in enumerate(convs):
            n = self.prompt_tokens(m)
            if n + 256 > self.max_model_len:
                out[i] = {"text": "", "usage": {"prompt_tokens": n, "completion_tokens": 0},
                          "finish_reason": "too_long"}
            else:
                fits.append((i, min(max_tokens, self.max_model_len - n)))
        params = [SamplingParams(temperature=temperature, top_p=top_p, top_k=top_k, max_tokens=mt,
                                 seed=int(seeds[i])) for i, mt in fits]
        results = self.llm.chat([convs[i] for i, _ in fits], params, use_tqdm=False) if fits else []
        for (i, _), r in zip(fits, results):
            c = r.outputs[0]
            out[i] = {"text": c.text, "finish_reason": c.finish_reason,
                      "usage": {"prompt_tokens": len(r.prompt_token_ids or []), "completion_tokens": len(c.token_ids)}}
        return out


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--model", required=True)
    ap.add_argument("--requests", required=True)
    ap.add_argument("--answers", required=True)
    ap.add_argument("--temperature", type=float, default=1.0)
    ap.add_argument("--top-p", type=float, default=0.95)
    ap.add_argument("--top-k", type=int, default=64)
    ap.add_argument("--max-tokens", type=int, default=12000)
    ap.add_argument("--max-model-len", type=int, default=int(os.environ.get("GEN_MAX_MODEL_LEN", "32768")))
    ap.add_argument("--gpu-memory-utilization", type=float, default=float(os.environ.get("GEN_GPU_MEM", "0.90")))
    a = ap.parse_args()

    reqs = [json.loads(l) for l in open(a.requests) if l.strip()]
    msgs = [r["messages"] if "messages" in r else
            [{"role": "system", "content": r["system"]}, {"role": "user", "content": r["user"]}] for r in reqs]
    llm = InProcessLLM(a.model, a.max_model_len, a.gpu_memory_utilization)
    print(f"llm_batch: {len(reqs)} prompts", flush=True)
    t1 = time.time()
    res = llm.generate(msgs, [r["seed"] for r in reqs], a.temperature, a.top_p, a.top_k, a.max_tokens)
    with open(a.answers + ".tmp", "w") as fh:
        for r, o in zip(reqs, res):
            fh.write(json.dumps({"id": r["id"], **o}) + "\n")
    os.replace(a.answers + ".tmp", a.answers)
    toks = sum(o["usage"]["completion_tokens"] for o in res)
    print(f"llm_batch: {len(reqs)} answers, {toks} tokens in {time.time() - t1:.0f} s "
          f"({toks / max(time.time() - t1, 1e-6):.0f} tokens/s)", flush=True)


if __name__ == "__main__":
    sys.exit(main())
