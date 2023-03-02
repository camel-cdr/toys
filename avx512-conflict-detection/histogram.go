package conflictdetection

const bits = 8
const bins = 1 << bits

// Procedure implements histogram schema showed in "Intel 64 and IA-32 Architectures
// Optimization Reference Manual", section "Example 18-19. Scatter Implementation
// Alternatives". Document revision: January 2023 (046A).

// go:noescape
// go:nosplit
func histogramIntelAsm(ptr *uint32, count uint64, output *uint32)

func histogramIntel(input []uint32, output []uint32) {
	histogramIntelAsm(&input[0], uint64(len(input)), &output[0])
}

// Procedure implements alternative histogram procedure

// go:noescape
// go:nosplit
func histogramV2Asm(ptr *uint32, count uint64, output *uint32)

func histogramV2(input []uint32, output []uint32) {
	histogramV2Asm(&input[0], uint64(len(input)), &output[0])
	if cap(output) != bins*16 {
		panic("wrong output size")
	}

	// the final step
	for i := 0; i < bins; i++ {
		pos := i * 16
		sum := uint32(0)
		for j := 0; j < 16; j++ {
			sum += output[pos+j]
		}

		output[i] = sum
	}
}

func histogramReference(input []uint32, output []uint32) {
	for _, idx := range input {
		output[idx] += 1
	}
}
