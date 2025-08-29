# 📊 Usage Guide — GraphXML

GraphXML lets you define charts in a simple **XML-based format**.

---

## 1. Example GraphXML File

Save this as `examples/example.graphxml`:

```xml
<graph type="line" title="Temperature by Days">
    <x-axis label="Days"/>
    <y-axis label="Temperature (°C)"/>
    
    <dataset name="City A" color="blue">
        <point x="Mon" y="20"/>
        <point x="Tue" y="22"/>
        <point x="Wed" y="19"/>
    </dataset>

    <dataset name="City B" color="red">
        <point x="Mon" y="18"/>
        <point x="Tue" y="21"/>
        <point x="Wed" y="17"/>
    </dataset>
</graph>


---

2. Running the Program

GUI Mode (Desktop)

./graphxml examples/example.graphxml

Opens an SDL2 window and renders the chart.


Headless Mode (Export to PNG)

./graphxml examples/example.graphxml output.png

Renders the chart directly into a PNG file without opening any window.

Very convenient for server or SSH environments.



---

3. Supported Graph Types

Currently available:

line → Line chart

bar → Bar chart

(More coming soon…)



---

4. API (C)

You can also use GraphXML as a library in your own C project:

#include "graphxml.h"

Graph g;
parse_graphxml("examples/example.graphxml", &g);

SDL_Init(SDL_INIT_VIDEO);
SDL_Window* win = SDL_CreateWindow("GraphXML",
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, 0);
SDL_Renderer* renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

render_graph(&g, renderer);

SDL_Delay(5000);
SDL_DestroyRenderer(renderer);
SDL_DestroyWindow(win);
SDL_Quit();

free_graph(&g);


---

5. File Extensions

Official extension: .graphxml

Alternative supported: .graph.xml



---

6. License

MIT
