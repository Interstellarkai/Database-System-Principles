from flask import Flask, request, render_template
from preprocessing import get_plans

app = Flask(__name__)


@app.after_request
def add_header(response):
    response.headers["X-UA-Compatible"] = "IE=Edge,chrome=1"
    response.headers["Cache-Control"] = "public, max-age=0"
    return response


@app.route("/", methods=["GET", "POST"])
def index():
    # context for rendering the page
    context = {
        "query": None,
        "errors": [],
        "plan_data": None,
        "summary_data": None,
    }

    # handle POST request
    if request.method == "POST":
        # get query from the POST request
        query = request.form["query"]

        if query:
            context["query"] = query

            # get query result
            has_error, result = get_plans(query)

            # add results to context
            if has_error:
                context["errors"].append(result["msg"])
            else:
                context["summary_data"] = result["summary_data"]
                context["plan_data"] = result["plan_data"]
                context["natural_explain"] = result["natural_explain"]
        else:
            context["errors"].append("No query provided!")

    return render_template("index.html", **context)


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, debug=True)
