---
title: Pulse
description: A lightweight uptime and status dashboard.
category: Web Apps
status: Active
year: 2026
role: Full-stack
featured: false
order: 3
links:
  repo: https://github.com/irbox/pulse
tags: [Node.js, SQLite, Monitoring]
features:
  - Concurrent multi-protocol health checks (HTTP, TCP, DNS)
  - Public status page with historical latency charts
  - Webhook alerts for Discord, Slack, and Email
stack:
  - { name: 'Node.js', purpose: 'Concurrent probe scheduler and daemon' }
  - { name: 'SQLite', purpose: 'Time-series telemetry storage' }
  - { name: 'Vanilla JS & Canvas', purpose: 'Lightweight latency sparkline rendering' }
gallery:
  - { caption: 'Public status overview' }
  - { caption: 'Latency percentile graphs' }
  - { caption: 'Incident reporting interface' }
---

Pulse is a self-hosted uptime and latency monitoring service designed for developers who want complete ownership over their infrastructure health checks.

## The problem

Third-party status monitoring SaaS tools typically charge significant recurring fees for basic minute-level pings, limit data retention, and lock public status pages behind expensive enterprise tiers.

## The approach

Pulse provides a lightweight, single-process monitoring daemon:

- **Worker Thread Probing**: Performs asynchronous HTTP, ICMP ping, and TLS certificate validation across dozens of endpoints without blocking event loop execution.
- **Embedded SQLite Storage**: Telemetry data is rolled up into hourly and daily percentiles (p50, p95, p99), keeping the database footprint under 20MB per year of continuous metrics.
- **Dependency-Free Status Page**: Public status dashboards render in under 10KB of HTML and CSS, ensuring the status page remains fully operational even during severe infrastructure degradation.

```bash
# Start Pulse with Docker or Node.js
git clone https://github.com/irbox/pulse.git
cd pulse && npm install
npm start
```

## Outcome

Operates reliably on low-cost 512MB VPS instances, monitoring 50+ endpoints with negligible memory and CPU overhead while dispatching instant downtime alerts.
