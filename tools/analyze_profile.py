#!/usr/bin/env python3
"""
ESP-IDF liboqs Profiling Analyzer

Parses profiling output from ESP32 logs and generates analysis reports.

Usage:
    # Capture logs
    idf.py monitor | tee log.txt
    
    # Analyze
    python analyze_profile.py log.txt
    
    # Or from stdin
    idf.py monitor | python analyze_profile.py -
"""

import re
import sys
import argparse
from collections import defaultdict
from typing import List, Dict, Tuple

class ProfileEntry:
    def __init__(self, name: str, calls: int, total_us: int, avg_us: int, min_us: int = 0, max_us: int = 0):
        self.name = name
        self.calls = calls
        self.total_us = total_us
        self.avg_us = avg_us
        self.min_us = min_us
        self.max_us = max_us
        self.percentage = 0.0

def parse_profile_log(log_lines: List[str]) -> Tuple[List[ProfileEntry], int]:
    """Parse profiling data from ESP32 log output"""
    
    entries = []
    rejection_count = 0
    
    # Pattern for profile lines with all fields
    pattern = r'liboqs_profile:\s+(\S+(?:\s+\S+)*?)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)'
    # Pattern for rejection count
    rejection_pattern = r'Signature rejection loop iterations:\s+(\d+)'
    
    in_profile_section = False
    
    for line in log_lines:
        # Check if we're in profiling section
        if 'PROFILING RESULTS' in line:
            in_profile_section = True
            continue
        
        if in_profile_section and '=====' in line:
            in_profile_section = False
            continue
        
        # Parse rejection count
        rej_match = re.search(rejection_pattern, line)
        if rej_match:
            rejection_count = int(rej_match.group(1))
        
        # Parse profile entries
        match = re.search(pattern, line)
        if match and in_profile_section:
            name = match.group(1).strip()
            calls = int(match.group(2))
            total_us = int(match.group(3))
            avg_us = int(match.group(4))
            min_us = int(match.group(5))
            max_us = int(match.group(6))
            
            entries.append(ProfileEntry(name, calls, total_us, avg_us, min_us, max_us))
    
    return entries, rejection_count

def calculate_percentages(entries: List[ProfileEntry]) -> None:
    """Calculate percentage of total time for each entry"""
    total_time = sum(e.total_us for e in entries)
    if total_time > 0:
        for entry in entries:
            entry.percentage = (entry.total_us / total_time) * 100.0

def print_analysis(entries: List[ProfileEntry], rejection_count: int) -> None:
    """Print detailed analysis of profiling data"""
    
    if not entries:
        print("No profiling data found in log")
        return
    
    calculate_percentages(entries)
    
    # Sort by total time
    entries_by_time = sorted(entries, key=lambda e: e.total_us, reverse=True)
    
    print("\n" + "="*100)
    print("PROFILING ANALYSIS REPORT")
    print("="*100)
    
    # Summary statistics
    total_time = sum(e.total_us for e in entries)
    total_calls = sum(e.calls for e in entries)
    
    print(f"\nSUMMARY:")
    print(f"  Total execution time: {total_time:,} μs ({total_time/1000:.3f} ms)")
    print(f"  Total function calls: {total_calls:,}")
    print(f"  Unique functions:     {len(entries)}")
    if rejection_count > 0:
        print(f"  Rejection iterations: {rejection_count}")
    
    # Top functions by total time
    print("\n" + "-"*100)
    print("TOP 10 FUNCTIONS BY TOTAL TIME:")
    print("-"*100)
    print(f"{'Function':<45} {'Calls':>10} {'Total(μs)':>15} {'Avg(μs)':>12} {'%':>8}")
    print("-"*100)
    
    for entry in entries_by_time[:10]:
        print(f"{entry.name:<45} {entry.calls:>10,} {entry.total_us:>15,} {entry.avg_us:>12,} {entry.percentage:>7.2f}%")
    
    # Top functions by call count
    entries_by_calls = sorted(entries, key=lambda e: e.calls, reverse=True)
    
    print("\n" + "-"*100)
    print("TOP 10 MOST CALLED FUNCTIONS:")
    print("-"*100)
    print(f"{'Function':<45} {'Calls':>10} {'Total(μs)':>15} {'Avg(μs)':>12}")
    print("-"*100)
    
    for entry in entries_by_calls[:10]:
        print(f"{entry.name:<45} {entry.calls:>10,} {entry.total_us:>15,} {entry.avg_us:>12,}")
    
    # Functions with highest variance (max - min)
    entries_with_variance = [e for e in entries if e.max_us > 0 and e.min_us > 0]
    entries_by_variance = sorted(entries_with_variance, 
                                key=lambda e: e.max_us - e.min_us, 
                                reverse=True)
    
    if entries_by_variance:
        print("\n" + "-"*100)
        print("TOP 10 FUNCTIONS WITH HIGHEST VARIANCE:")
        print("-"*100)
        print(f"{'Function':<45} {'Min(μs)':>12} {'Max(μs)':>12} {'Variance':>12} {'Avg(μs)':>12}")
        print("-"*100)
        
        for entry in entries_by_variance[:10]:
            variance = entry.max_us - entry.min_us
            print(f"{entry.name:<45} {entry.min_us:>12,} {entry.max_us:>12,} {variance:>12,} {entry.avg_us:>12,}")
    
    # Categorization by function type
    categories = defaultdict(lambda: {'time': 0, 'calls': 0})
    
    for entry in entries:
        name_lower = entry.name.lower()
        if 'ntt' in name_lower:
            cat = 'NTT Operations'
        elif 'matrix' in name_lower:
            cat = 'Matrix Operations'
        elif 'shake' in name_lower or 'sha' in name_lower:
            cat = 'Hashing (SHAKE/SHA)'
        elif 'uniform' in name_lower or 'challenge' in name_lower:
            cat = 'Sampling'
        elif 'pack' in name_lower or 'unpack' in name_lower:
            cat = 'Packing/Unpacking'
        elif 'keypair' in name_lower:
            cat = 'Key Generation'
        elif 'sign' in name_lower:
            cat = 'Signing'
        elif 'verify' in name_lower:
            cat = 'Verification'
        else:
            cat = 'Other'
        
        categories[cat]['time'] += entry.total_us
        categories[cat]['calls'] += entry.calls
    
    print("\n" + "-"*100)
    print("TIME BREAKDOWN BY CATEGORY:")
    print("-"*100)
    print(f"{'Category':<30} {'Total(μs)':>15} {'Total(ms)':>15} {'%':>8} {'Calls':>10}")
    print("-"*100)
    
    for cat, data in sorted(categories.items(), key=lambda x: x[1]['time'], reverse=True):
        pct = (data['time'] / total_time) * 100.0
        print(f"{cat:<30} {data['time']:>15,} {data['time']/1000:>15.3f} {pct:>7.2f}% {data['calls']:>10,}")
    
    print("="*100)
    
    # Optimization suggestions
    print("\nOPTIMIZATION SUGGESTIONS:")
    print("-"*100)
    
    # Find hotspots (functions taking >10% of time)
    hotspots = [e for e in entries_by_time if e.percentage > 10.0]
    if hotspots:
        print("\n🔥 HOTSPOTS (>10% of total time):")
        for entry in hotspots:
            print(f"   - {entry.name}: {entry.percentage:.1f}% ({entry.total_us/1000:.1f} ms)")
            
            # Specific suggestions
            name_lower = entry.name.lower()
            if 'ntt' in name_lower:
                print(f"     → Consider: Hardware acceleration, optimized NTT algorithms")
            elif 'matrix' in name_lower:
                print(f"     → Consider: SIMD optimization, better memory access patterns")
            elif 'shake' in name_lower:
                print(f"     → Consider: Hardware SHA3 acceleration (ESP32-C6/S3)")
    
    # Check rejection rate
    if rejection_count > 0 and total_calls > 0:
        # Estimate number of signatures (rough heuristic)
        sign_calls = sum(e.calls for e in entries if 'sign_internal' in e.name.lower())
        if sign_calls > 0:
            avg_rejections = rejection_count / sign_calls
            print(f"\n📊 REJECTION STATISTICS:")
            print(f"   - Average rejections per signature: {avg_rejections:.2f}")
            if avg_rejections > 6:
                print(f"     ⚠️  High rejection rate! Expected: ~4.5")
            elif avg_rejections < 3:
                print(f"     ⚠️  Unusually low rejection rate")
            else:
                print(f"     ✓  Normal rejection rate (expected ~4.5)")
    
    print("\n" + "="*100 + "\n")

def main():
    parser = argparse.ArgumentParser(
        description='Analyze ESP-IDF liboqs profiling results',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Analyze from log file
  python analyze_profile.py monitor_log.txt
  
  # Live monitoring
  idf.py monitor | tee log.txt
  python analyze_profile.py log.txt
  
  # Analyze from stdin
  idf.py monitor | python analyze_profile.py -
        """
    )
    
    parser.add_argument('logfile', 
                       help='Log file to analyze (use - for stdin)')
    parser.add_argument('-o', '--output',
                       help='Save report to file')
    
    args = parser.parse_args()
    
    # Read input
    if args.logfile == '-':
        lines = sys.stdin.readlines()
    else:
        try:
            with open(args.logfile, 'r') as f:
                lines = f.readlines()
        except FileNotFoundError:
            print(f"Error: File '{args.logfile}' not found")
            return 1
    
    # Parse and analyze
    entries, rejection_count = parse_profile_log(lines)
    
    if not entries:
        print("No profiling data found in log file.")
        print("Make sure:")
        print("  1. CONFIG_LIBOQS_ENABLE_PROFILING is enabled")
        print("  2. Your code calls esp_liboqs_profile_print()")
        print("  3. The log contains profiling output")
        return 1
    
    # Print or save analysis
    if args.output:
        original_stdout = sys.stdout
        with open(args.output, 'w') as f:
            sys.stdout = f
            print_analysis(entries, rejection_count)
        sys.stdout = original_stdout
        print(f"Analysis saved to {args.output}")
    else:
        print_analysis(entries, rejection_count)
    
    return 0

if __name__ == '__main__':
    sys.exit(main())
