# NetworkAnalysis

A C++ network analysis tool that provides network topology generation, flow optimization algorithms, and analysis capabilities through an interactive shell interface.

If you use this project in a paper, please cite:

Improving an immune-inspired algorithm by linear regression: A case study on network reliability
(https://doi.org/10.1016/j.knosys.2024.112034)


## Features

### Network Management
- **Network Generation**: Create networks with customizable parameters including:
  - Node count and layer structure
  - Edge capacity ranges
  - Edge count per node
  - Entropy settings
- **Topology Operations**: Load, save, and manipulate network topologies
- **Network Modifications**: 
  - Full connection of networks
  - Pruning operations
  - MATLAB script export

### Flow Solver Algorithms
The project implements multiple flow optimization algorithms:

1. **Immune Algorithm** (`IMMUNE`) - Bio-inspired optimization
2. **Ant Colony Optimization** (`ACO`) - Swarm intelligence approach  
3. **Ford-Fulkerson Algorithm** (`FORD_FUKERSON`) - Classic maximum flow algorithm

### Interactive Shell
Command-line interface supporting:
- Network operations (open, generate, save, export)
- Flow solver configuration and execution
- Batch script execution
- Real-time network analysis

## Project Structure

```
NetworkAnalysis/
├── main.cpp                 # Main application and shell commands
├── compile-debug.bat        # Debug build script
├── compile-release.bat      # Release build script
├── core/                    # Core utilities and shell
│   ├── core.h              # Main core header
│   ├── shell.cpp/.h        # Interactive shell implementation
│   ├── utility.cpp/.h      # Utility functions
│   ├── mathex.hpp          # Mathematical extensions
│   ├── random.hpp          # Random number generation
│   └── QQplotGenerator.*   # Statistical plotting
├── flowsolver/              # Flow optimization algorithms
│   ├── flowsolverbase.*    # Base solver interface
│   ├── acosolver.*         # Ant Colony Optimization
│   ├── immunesolver.*      # Immune Algorithm
│   └── ffsolver.*          # Ford-Fulkerson Algorithm
├── network/                 # Network and topology management
│   ├── network.*           # Network data structures
│   └── topology.*          # Topology operations
└── bin/debug/              # Compiled object files
```

## Building

### Prerequisites
- GCC compiler with C++11 support
- Windows environment (batch scripts provided)

### Debug Build
```bash
./compile-debug.bat
```

### Release Build
```bash
./compile-release.bat
```

## Usage

### Interactive Mode
Run the executable without arguments to start the interactive shell:
```bash
./networkanalysis.exe
```

### Script Mode
Execute commands from a script file:
```bash
./networkanalysis.exe -f script.txt
```

### Shell Commands

#### Network Operations
- `network.open <filename>` - Load network from file
- `network.generate [options]` - Generate network with parameters:
  - `capacityEdge=<value>` - Edge capacity range
  - `countEdgeForNode=<value>` - Edges per node
  - `countLayer=<value>` - Number of layers
  - `countNode=<value>` - Total nodes
  - `deltaNodeLayer=<value>` - Node distribution per layer
  - `entropy=<value>` - Randomness factor
- `network.save <filename>` - Save network topology
- `network.fullconnected` - Make network fully connected
- `network.pruning` - Remove unnecessary edges
- `network.export` - Export as MATLAB script
- `network.print` - Display network information

#### Flow Solver Operations
- `flowsolver.create <method>` - Create solver (0=Immune, 1=ACO, 2=Ford-Fulkerson)
- `flowsolver.run` - Execute flow optimization
- `flowsolver.dispose` - Clean up solver

## Algorithm Details

### Immune Algorithm
Bio-inspired optimization algorithm that mimics the adaptive immune system for finding optimal network flows.

### Ant Colony Optimization (ACO)
Swarm intelligence algorithm where artificial ants find optimal paths through pheromone-based communication.

### Ford-Fulkerson Algorithm
Classical maximum flow algorithm that finds the maximum possible flow from source to sink in a flow network.

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test with both debug and release builds
5. Submit a pull request


## Technical Notes

- The project uses a modular architecture with separate namespaces for core utilities, graph operations, and flow solvers
- Memory management is handled manually with careful allocation/deallocation
- The shell interface supports both interactive and batch processing modes
- Object files are organized in the `bin/debug/` directory for clean builds