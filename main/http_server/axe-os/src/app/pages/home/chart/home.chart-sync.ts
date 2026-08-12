// home.chart-sync.ts

import { applyHashrate1mSmoothing } from './home.chart-smoothing';
import type { Hashrate1mSmoothingCfg } from './home.chart-smoothing';
import type { ChartZoomCfg } from './home.chart-zoom';
import type { HomeChartSeriesRefs } from './home.chart.datasets';

export interface SyncHomeChartDataOptions {
  chartData: any;
  chartOptions?: any;
  chart?: any;
  series: HomeChartSeriesRefs;
  hashrateDisplayValue?: number;
  smoothingCfg: Hashrate1mSmoothingCfg;
  windowMs: number;
  zoomCfg: ChartZoomCfg;
}

export function syncHomeChartDataAndSmoothing(opts: SyncHomeChartDataOptions): void {
  if (!opts || !opts.chartData || !opts.series) return;

  const chartData = opts.chart?.data ?? opts.chartData;
  const chartOptions = opts.chart?.options ?? opts.chartOptions;
  const { series } = opts;

  const OFFSET = 0.4;  

  chartData.labels = series.labels;
  const datasets: any[] = Array.isArray(chartData.datasets) ? chartData.datasets : [];

  if (datasets.length >= 6) {
  
    const applyOffset = (data: number[]) => 
      data.map(v => {
        const val = Number(v);
        return Number.isFinite(val) ? val + OFFSET : val;
      });


    datasets[0].data = applyOffset(series.hr1m);   
    datasets[1].data = applyOffset(series.hr10m);  
    datasets[2].data = applyOffset(series.hr1h);  
    datasets[3].data = series.hr1d; 
    
    datasets[4].data = series.vregTemp;
    datasets[5].data = series.asicTemp;
  }

  // Update hashrate pill display value from live pool sum.
  if (chartOptions?.plugins?.valuePills && Number.isFinite(opts.hashrateDisplayValue as any)) {
    chartOptions.plugins.valuePills.hashrateDisplayValue = opts.hashrateDisplayValue;
  }


  try {
    const ds: any = datasets.length ? datasets[0] : null;
    //applyHashrate1mSmoothing(ds, series.labels || [], opts.smoothingCfg, opts.windowMs, opts.zoomCfg);
    applyHashrate1mSmoothing(ds, series.labels || [], opts.smoothingCfg, opts.windowMs, opts.zoomCfg, 0.4);
  } catch {}
}
