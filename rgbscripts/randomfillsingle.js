// Development tool access
var testAlgo;

(
    /**
     * This algorithm produces RGB maps where exactly one pixel is lit at a time
     * on a single map (=step). The pixel is chosen at random for each step, ensuring
     * that each pixel appears exactly once per round ($width * $height steps).
     */
    function()
    {
        var algo = new Object;
        algo.apiVersion = 1;
        algo.name = "Random Fill Single";
        algo.author = "plg";
        algo.width = 0;
        algo.height = 0;

        var util = new Object;
        /**
         * Create an array of length($width * height), with each item
         * containing a sub-array representing one point (y, x) in a map.
         */
        util.createStepList = function(width, height)
        {
            var list = new Array(height * width);
            var i = 0;
            for (var y = 0; y < height; y++)
            {
                for (var x = 0; x < width; x++)
                {
                    list[i] = [y, x];
                    i++;
                }
            }

            return list;
        }

        /**
         * Create one full map of size($width, $height), where exactly
         * one cell($sx, $sy) is active. Leave other cells black (0).
         */
        util.createStep = function(width, height, sy, sx, lastStep)
        {
            var map = new Array(height);
            for (var y = 0; y < height; y++)
            {
                map[y] = new Array(width);
                for (var x = 0; x < width; x++)
                {
                    if (sy == y && sx == x)
                        map[y][x] = 1;
                    else if (lastStep != 0 && lastStep[y][x] == 1)
                        map[y][x] = 1;
                    else
                        map[y][x] = 0;
                }
            }

            return map;
        }

        /**
         * Create map with the real rgb value (because it now changes at each step)
         */
        util.createStepRgb = function(width, height, step, rgb)
        {
            var map = new Array(height);
            for (var y = 0; y < height; y++)
            {
                map[y] = new Array(width);
                for (var x = 0; x < width; x++)
                {
                    if (step[y][x] != 0)
                        map[y][x] = rgb;
                    else
                        map[y][x] = 0;
                }
            }

            return map;
        }

        /**
         * The actual "algorithm" for this RGB script. Produces a map of
         * size($width, $height) each time it is called.
         *
         * @param step The step number that is requested (0 to (algo.rgbMapStepCount - 1))
         * @param rgb Tells the color requested by user in the UI.
         * @return A two-dimensional array[height][width].
         */
        algo.rgbMap = function(width, height, rgb, step)
        {
            // Create a new step list only when attributes change to keep the
            // script running with as little extra overhead as possible.
            // Create a new step list each new iteration of the loop so it does not look repetitive
            if (algo.width != width || algo.height != height || parseInt(step) == 0)
            {
                // To ensure that ALL points are included in the steps exactly
                // once, a list of possible steps is created (from (0,0) to (w-1,h-1)).
                var stepList = util.createStepList(width, height);

                algo.steps = new Array(width * height);
                var lastStep = 0;
                for (var i = 0; i < width * height; i++)
                {
                    // Pick a random index from the list of possible steps. The
                    // item in the list tells the actual map point that is lit
                    // in this step.
                    var index = Math.floor(Math.random() * (stepList.length));
                    var yx = stepList[index];
                    var map = util.createStep(width, height, yx[0], yx[1], lastStep);
                    algo.steps[i] = map;
                    lastStep = map;

                    // Remove the used item from the list of possible steps so that
                    // it won't be picked again.
                    stepList.splice(index, 1);
                }

                algo.width = width;
                algo.height = height;
            }

            // refresh with real color

            return util.createStepRgb(width, height, algo.steps[step], rgb);
        }

        /**
         * Tells RGB Matrix how many steps this algorithm produces with size($width, $height)
         *
         * @param width The width of the map
         * @param height The height of the map
         * @return Number of steps required for a map of size($width, $height)
         */
        algo.rgbMapStepCount = function(width, height)
        {
            // All pixels in the map must be used exactly once, each one separately
            // at a time. Therefore, the maximum number of steps produced by this
            // script on a 5 * 5 grid is 25.
            return width * height;
        }

        // Development tool access
        testAlgo = algo;

        return algo;
    }
)()
